FROM node:latest

# Install g++
RUN apt-get update && apt-get install -y g++

# Install build-essential for compiling binaries
RUN apt-get install -y build-essential

WORKDIR /

COPY package.json ./

RUN npm install

RUN npm install -g nodemon

COPY . .

RUN g++ -o cacheSimulator/testCacheSim cacheSimulator/testCacheSim.cpp

RUN chmod +x /cacheSimulator/testCacheSim

EXPOSE 80 8080

CMD ["node", "app.js"]



