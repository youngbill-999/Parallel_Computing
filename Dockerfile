FROM tudinfse/cds_server
ADD mopp-2018-t0-harmonic-progression-sum  /tmp/test
ADD kmeans /tmp/kmeans
ADD mandelbrot /tmp/mandelbrot
ADD cds_server.json /etc/cds_server.json
RUN apt update && \
    apt install -y gcc g++ make 
RUN cd /tmp/test  && \
    make
RUN cd /tmp/kmeans  && \
    make
RUN cd /tmp/mandelbrot && \ 
    make
    
ENV CDS_PORT 8080
ENV RUST_LOG debug
CMD cds_server -c /etc/cds_server.json -p $CDS_PORT