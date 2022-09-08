src = $(wildcard ./*.cpp)
objects = $(patsubst %.cpp, %.o, $(src))
target = app
$(target) : $(objects)
	$(CXX) $(objects) -o $(target)
	
%.o:%.cpp
	$(CXX) -c $< -o  $@

.PHONY:clean
clean:
	rm $(objects) -f
