//
// Created by bladerunner on 10/15/26.
//

#ifndef HOMEWORKONE_3_TESTRUNNER_H
#define HOMEWORKONE_3_TESTRUNNER_H


#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include <exception>
#include <iostream>
#include <stdexcept>

namespace tests
{
    inline int total = 0;
    inline int passed = 0;

    inline void check(bool condition, const char* description)
    {
        if (!condition)
        {
            throw std::runtime_error(description);
        }
    }

    inline void run(const char* description, void (*test)())
    {
        ++total;

        std::cout << "Тест №" << total
                  << ": " << description << '\n';

        try
        {
            test();
            ++passed;

            std::cout << "[pass]\n\n";
        }
        catch (const std::exception& error)
        {
            std::cout << "[failed] "
                      << error.what() << "\n\n";
        }
        catch (...)
        {
            std::cout << "[failed] Неизвестное исключение\n\n";
        }
    }

    inline int summary()
    {
        std::cout << "Тестов пройдено: "
                  << passed << '/' << total << std::endl;

        return passed == total ? 0 : 1;
    }
}

#endif
#endif //HOMEWORKONE_3_TESTRUNNER_H
