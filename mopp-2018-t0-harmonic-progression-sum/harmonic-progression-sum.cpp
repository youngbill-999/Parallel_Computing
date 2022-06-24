#include <iostream>
#include <sstream>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/sysinfo.h>

using namespace std;

struct thread_work_t{
    long unsigned d;
    long unsigned start_n;
    long unsigned end_n;
    long unsigned int *digits;
};

void *sum(void *thread_work_uncasted) {
    struct thread_work_t *thread_work = (struct thread_work_t*)thread_work_uncasted;
    const long unsigned int d = thread_work->d;
    const long unsigned int start_n = thread_work->start_n;
    const long unsigned int end_n = thread_work->end_n;
    long unsigned int *digits = thread_work->digits;

    // intermediate value
    for (long unsigned int digit = 0; digit < d + 11; ++digit) {
        digits[digit] = 0;//here defines only 11 digits ,why?
    }
    // main loop, calculate for each i the value of 1/i to a precesion(精度) of d
    for (long unsigned int i = start_n; i <= end_n; ++i) {
        long unsigned int remainder = 1;
        for (long unsigned int digit = 0; digit < d + 11 && remainder; ++digit) {
            long unsigned int div = remainder / i;
            long unsigned int mod = remainder % i;
            digits[digit] += div;
            remainder = mod * 10;
        }
    }
    return 0;
}

void generate_output(long unsigned int *digits, int d, char* output) {
    // move values bigger than 10 up
    for (long unsigned int i = d + 11 - 1; i > 0; --i) {
        digits[i - 1] += digits[i] / 10;
        digits[i] %= 10;
    }
    // round last digit
    if (digits[d + 1] >= 5) {
        ++digits[d];
    }
    // move values bigger than 10 up
    for (long unsigned int i = d; i > 0; --i) {
        digits[i - 1] += digits[i] / 10;
        digits[i] %= 10;
    }
    // print result into output
    stringstream stringstreamA;
    stringstreamA << digits[0] << ".";
    for (long unsigned int i = 1; i <= d; ++i) {
        stringstreamA << digits[i];
    }
    stringstreamA << '\0';
    string stringA = stringstreamA.str();
    stringA.copy(output, stringA.size());
}

int main() {
    // Determine the amount of available CPUs
    int cpus = get_nprocs();//linux下的功能用以获取cpu核数
    // nprocs() might return wrong amount inside of a container.
    // Use MAX_CPUS instead, if available.
    if (getenv("MAX_CPUS")) {
        cpus = atoi(getenv("MAX_CPUS"));//getenv把环境变量“MAX_CPUS”转换成一个字符串，多数情况是一个上下文路径，atoi把这个路径转换为一个整型
    }
    // Sanity-check
    assert(cpus > 0 && cpus <= 64);//相当于判断，如果fail则终止程序
    fprintf(stderr, "Running on %d CPUs\n", cpus);

    long unsigned int d = 1, n = 1; //cpus = 1;有问题？？？？？？？？？？？？？？？？？？？？

    // read input
    cin >> d >> n;//为何这里de cpus是1

    long unsigned int digits[cpus][d+11];
    struct thread_work_t tw[cpus];
    pthread_t thread[cpus];//定义线程的 id 变量，多个变量使用数组

    for (int i=0; i < cpus; i++) {

        tw[i].digits  = digits[i];//tw[i].digits是指的没得分线程里的操作结果，对应于二维总结果中的一个分行结果
        tw[i].start_n = (n/cpus)* i    + 1;
        tw[i].end_n   = (n/cpus)*(i+1);
        tw[i].d       = d;
        //以上是每一个分线程的输入数据的初始化

        fprintf(stderr, "Starting thread %d from %ld to %ld\n", i, tw[i].start_n, tw[i].end_n);
    
        pthread_create(&thread[i], NULL, sum, (void*)&tw[i]);//pthread_create (线程id, 线程参数, 调用的函数, 调用函数的参数) 
    }

    // main thread takes care of the leftover主线程处理剩余的
    struct thread_work_t maintw;
    long unsigned int mdigits[d+11] = {0};
    maintw.digits  = mdigits;
    maintw.start_n = (n/cpus) * cpus + 1;
    maintw.end_n   = n;
    maintw.d       = d;

    sum((void*)&maintw);//？？？？？？？？？？？？？？？？

    for (int i=0; i<cpus; i++) {
        // wait for all threads
        pthread_join(thread[i], NULL);//线程连接，当分线程thread[i],执行完毕后可以执行下面的操作
        for (int j=0; j<=d+11; j++) {
            mdigits[j] += tw[i].digits[j];
        }
    }

    // allocate output buffer
    char output[d + 10]; // extra precision due to possible error
    generate_output(mdigits, d, output);//调用上面的函数，输出结果

    cout << output << endl;

    return 0;
}
