#include "../alpha/alpha.h"
#include "../alpha/objectPool.h"

alphaMin::Logger::ptr logger = ALPHA_LOG_ROOT();

class MyObject {
public:
    typedef std::shared_ptr<MyObject> ptr;
    MyObject() {
        ALPHA_LOG_INFO(logger) << "构造 myObject";
    }

    ~MyObject() {
        ALPHA_LOG_INFO(logger) << "析构 myObject";
    }

    int value = 0;
};

void test(alphaMin::ObjectPool<MyObject>& pool) {
    auto obj1 = pool.get();
    ALPHA_LOG_INFO(logger) << "pool's size = " << pool.get_size() << " maxSize = " << pool.get_maxSize();
    auto obj2 = pool.get();
    ALPHA_LOG_INFO(logger) << "pool's size = " << pool.get_size() << " maxSize = " << pool.get_maxSize();
    auto obj3 = pool.get();
    ALPHA_LOG_INFO(logger) << "pool's size = " << pool.get_size() << " maxSize = " << pool.get_maxSize();
    auto obj4 = pool.get();
    ALPHA_LOG_INFO(logger) << "pool's size = " << pool.get_size() << " maxSize = " << pool.get_maxSize();
    auto obj5 = pool.get();
    ALPHA_LOG_INFO(logger) << "pool's size = " << pool.get_size() << " maxSize = " << pool.get_maxSize();
    auto obj6 = pool.get();
    ALPHA_LOG_INFO(logger) << "pool's size = " << pool.get_size() << " maxSize = " << pool.get_maxSize();

    ALPHA_LOG_INFO(logger) << "============================================";

    pool.put(obj1);
    ALPHA_LOG_INFO(logger) << "pool's size = " << pool.get_size() << " maxSize = " << pool.get_maxSize();
    pool.put(obj2);
    ALPHA_LOG_INFO(logger) << "pool's size = " << pool.get_size() << " maxSize = " << pool.get_maxSize();
    pool.put(obj3);
    ALPHA_LOG_INFO(logger) << "pool's size = " << pool.get_size() << " maxSize = " << pool.get_maxSize();
    pool.put(obj4);
    ALPHA_LOG_INFO(logger) << "pool's size = " << pool.get_size() << " maxSize = " << pool.get_maxSize();
    pool.put(obj5);
    ALPHA_LOG_INFO(logger) << "pool's size = " << pool.get_size() << " maxSize = " << pool.get_maxSize();
    pool.put(obj6);
    ALPHA_LOG_INFO(logger) << "pool's size = " << pool.get_size() << " maxSize = " << pool.get_maxSize();

    ALPHA_LOG_INFO(logger) << "============================================";
    auto obj7 = pool.get();
    ALPHA_LOG_INFO(logger) << "pool's size = " << pool.get_size() << " maxSize = " << pool.get_maxSize();

    ALPHA_LOG_INFO(logger) << "============================================";

    MyObject::ptr obj(new MyObject);
    pool.put(obj);

    ALPHA_LOG_INFO(logger) << "============================================";

    pool.close();

    ALPHA_LOG_INFO(logger) << "pool's size = " << pool.get_size() << " maxSize = " << pool.get_maxSize();
}

int main(int argc, char** argv) {

    alphaMin::ObjectPool<MyObject> pool(5);

    alphaMin::IOManager iom(2);

    

    iom.schedule([&pool]() {
        test(pool);
    });

    return 0;
}