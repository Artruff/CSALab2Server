FROM ubuntu:latest

RUN apt-get clean && \
    apt-get update --allow-releaseinfo-change && \
    apt-get install -y cmake git sqlite3 libsqlite3-dev build-essential

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

RUN make clean
# Сборка проекта
RUN make all
# Открытие порта
EXPOSE 8080
ENV LD_LIBRARY_PATH=/usr/src/app/lib:$LD_LIBRARY_PATH

#RUN /usr/src/app/build/CSALab2
# Запуск приложения
CMD ["/usr/src/app/build/CSALab2"]
