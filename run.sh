command='g++ ./src/main.cpp -o main -lpng -std=c++11'
command+=' -O3'
command+=' -g3'
command+=' -fopenmp'

touch render.png
xdg-open render.png
$command
./main
