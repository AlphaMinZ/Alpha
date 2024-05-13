clone 本项目后，进入到 Alpha 目录下，你需要

mkdir bin

mkdir build

cd build

cmake ..

make

tests 目录里面的测试文件的可执行文件会编译在 bin 文件下

最新更新了 Chan 结构，如果是阻塞通道，发送端在 chan 满时会陷入阻塞，在其他协程读取数据之后唤醒他，读之亦然。对于非阻塞 chan ，满时写段会直接返回写失败，空时读端会返回空指针数据表示 chan 为空。
