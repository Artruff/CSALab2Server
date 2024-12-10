FROM ubuntu:latest

RUN apt-get clean && \
    apt-get update --allow-releaseinfo-change && \
    apt-get install -y cmake git sqlite3 libsqlite3-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Установка libhv
RUN git clone --recurse-submodules https://github.com/ithewei/libhv.git /usr/src/libhv && \
    cd /usr/src/libhv && \
    mkdir build && cd build && \
    cmake .. && \
    make && \
    make install

# Установка Catch2
RUN apt install -y catch

# Рабочая директория
WORKDIR /usr/src/app

# Копирование файлов
COPY . .

# Сборка проекта
RUN make all

# Открытие порта
EXPOSE 8080

# Запуск приложения
CMD [".build/CSALab2"]
