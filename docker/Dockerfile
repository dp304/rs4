FROM debian:buster

RUN apt-get update &&\
    apt-get install -y sudo git libtool g++ cmake python3-pip &&\
    pip3 install conan &&\
    conan profile new default --detect &&\
    conan profile update settings.compiler.libcxx=libstdc++11 default &&\
    conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan

COPY ./build.sh /bin

CMD ["/bin/bash", "/bin/build.sh"]
