#include "base/stdafx.h"

class Printer
{
private:
    asio::strand<asio::io_context::executor_type> strand_;
    asio::steady_timer timer1_;
    asio::steady_timer timer2_;
    int count_;

public:
    // strand类模板是一个执行器适配器，它保证通过它分派的处理程序，在前一个处理程序完成执行后才会启动下一个。
    // 无论有多少线程正在调用asio::io_context::run()，这个保证都成立。当然，这些处理程序仍可能与其他未通过strand分派或通过不同strand对象分派的处理程序并发执行。
    Printer(asio::io_context &io) :
        strand_(asio::make_strand(io)),
        timer1_(io, asio::chrono::seconds(1)),
        timer2_(io, asio::chrono::seconds(2)),
        count_(0)
    {

        // 在启动异步操作时，每个完成处理程序都被"绑定"到一个asio::strand<asio::io_context::executor_type>对象。
        // asio::bind_executor()函数返回一个新的处理程序，它会自动通过strand对象分派其包含的处理程序。通过将处理程序绑定到同一个strand，我们确保它们不会并发执行。
        timer1_.async_wait(asio::bind_executor(strand_, std::bind(&Printer::print1, this)));
        timer2_.async_wait(asio::bind_executor(strand_, std::bind(&Printer::print2, this)));
    }

    ~Printer()
    {
        std::cout << "final count: " << count_ << std::endl;
    }

    void print1()
    {
        if (count_ < 10)
        {
            std::cout << "Timer 1:" << count_ << std::endl;
            ++count_;

            timer1_.expires_at(timer1_.expiry() + asio::chrono::seconds(1));
            timer1_.async_wait(asio::bind_executor(strand_, std::bind(&Printer::print1, this)));
        }
    }

    void print2()
    {
        if (count_ < 10)
        {
            std::cout << "Timer 2:" << count_ << std::endl;
            ++count_;

            timer2_.expires_at(timer2_.expiry() + asio::chrono::seconds(1));
            timer2_.async_wait(asio::bind_executor(strand_, std::bind(&Printer::print2, this)));
        }
    }
};

void run_timer2()
{
    asio::io_context io;
    Printer p(io);
    std::thread t([&]()
                  {
                      io.run();
                  });
    io.run();
    t.join();
}