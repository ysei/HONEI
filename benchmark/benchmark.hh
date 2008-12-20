/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include <honei/util/stringify.hh>
#include <honei/util/exception.hh>
#include <honei/util/tags.hh>
#include <honei/util/benchmark_info.hh>
#include <string> 
#include <exception>
#include <time.h>
#include <sys/time.h>
#include <iostream>


/**
 * Baseclass for all testingclasses
 */
class Benchmark
{
    protected:
        const std::string _id;
        timeval _start, _end;
        std::list<double> _benchlist;
        int _x, _xmin, _xmax;
        double _total, _min, _max, _avg, _median, _tp, _mediantp, _f, _medianf, _mintp, _minf;
        std::string _tag_name;
        bool _plots;


    public:
        /**     
         * Constructor
         * \param id the Benchmark
         */
        Benchmark(const std::string & id);

        virtual ~Benchmark()
        {
        }

        const std::string id() const;

        /// called by the benchmark framework to run the benchmark
        virtual void run() = 0;

        void calculate();

        void calculate(BenchmarkInfo info);

        /// generates a standard benchmark output
        void evaluate();

        /// generates a benchmark output with FLOPS and data transfer rate information.
        void evaluate(BenchmarkInfo info);

        void evaluate_to_plotfile(std::list<BenchmarkInfo> info, std::list<std::string> cores, int count);

        /// Register our target platform.
        virtual void register_tag(std::string tag_name);

        /// Returns our target platform.
        virtual std::string get_tag_name();

        virtual bool plots();
};

/**
 * Exception thrown by Benchmarks
 */
class BenchFailedException :
    public std::exception
{
    private:
        std::string _message;

    public:
        /**
         * Constructor.
         */
        BenchFailedException(const char * const function, const char * const file,
            const long line, const std::string & message) throw ()
        {
            _message = message;
        }

        /**
         * Destructor.
         */
        virtual ~BenchFailedException() throw ()
        {
        }

        /**
         * Description.
         */
        const char * what() const throw ()
        {
            return _message.c_str();
        }
};

/**
 * Benchmarks and adds result to list
 */
#define BENCHMARK(a) \
    do { \
        gettimeofday(&_start, 0); \
        a; \
        gettimeofday(&_end, 0); \
        _benchlist.push_back(((double)_end.tv_sec + ((double)_end.tv_usec/1000000)) - ((double)_start.tv_sec + ((double)_start.tv_usec/1000000))); \
    } while (false)

