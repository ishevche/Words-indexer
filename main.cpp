#include <oneapi/tbb/flow_graph.h>
#include <iostream>
#include <filesystem>

namespace fg = tbb::flow;

struct MyStruct {
    size_t number;
};

int main() {
    fg::graph g;
    fg::function_node < size_t, std::shared_ptr<MyStruct> > echo(
            g, fg::unlimited,
            [](size_t a) {
                return std::make_shared<MyStruct>(MyStruct(a));
            }
    );
    fg::buffer_node<std::shared_ptr<MyStruct>> buffer(g);
    fg::limiter_node<std::shared_ptr<MyStruct>> limiter(g, 1);
    fg::function_node < std::shared_ptr<MyStruct>, fg::continue_msg > print(
            g, 1,
            [&](const std::shared_ptr<MyStruct> &aStruct) {
                std::cout << aStruct->number << std::endl;
                return fg::continue_msg();
            }
    );
    fg::make_edge(echo, buffer);
    fg::make_edge(buffer, limiter);
    fg::make_edge(limiter, print);

    echo.try_put(1);
    echo.try_put(2);
    echo.try_put(3);
    echo.try_put(4);
    echo.try_put(5);

    g.wait_for_all();

    std::shared_ptr<MyStruct> a;
    std::cout << "Left in buffer" << std::endl;
    while (buffer.try_get(a)) {
        std::cout << a->number << std::endl;
    }
}
