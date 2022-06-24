NAME		=	mopp-2017-t4-k-means-clustering    
LD			=	$(CXX)
DEBUG		+=	#-DDEBUG
CFLAGS		+=  -o3 -lpthread $(DEBUG)
CXXFLAGS	+=	$(CFLAGS)
LDFLAGS		+=

EXE			=	$(NAME)
CFILES		=	$(wildcard *.c)
CXXFILES	=	$(wildcard *.cpp)
OBJECTS		=	$(CFILES:.c=.o) $(CXXFILES:.cpp=.o)

all : $(EXE)
	@echo "Building..."

$(EXE) : $(OBJECTS)
	$(LD) $^ $(CFLAGS) $(LDFLAGS) -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	rm -fr $(EXE) $(OBJECTS)

run: $(EXE)
	./$(EXE) < input.txt > output.txt
