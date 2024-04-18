# Use a base image with necessary dependencies for C++ and OpenMP
FROM gcc:latest

# Set the working directory inside the container
WORKDIR /app

# Copy the source code into the container
COPY . .

# Compile the program with OpenMP support
RUN g++ -O2 -fopenmp -o solution solution.cpp

# Define the entry point for the container
ENTRYPOINT ["./solution"]