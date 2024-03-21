/*
 * File:   main.cpp
 * Date:   July 7, 2023
 * Brief:  Sample program to test various implementations of the CholeskyQR2-based QR factorization.
 * 
 * This file is part of the CholeskyQR2++ library.
 * 
 * Copyright (c) 2023-2024 Centre for Informatics and Computing,
 * Rudjer Boskovic Institute, Croatia. All rights reserved.
 * 
 * License: 3-clause BSD (BSD License 2.0)
 */



#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <boost/program_options.hpp>

#ifdef GSCHOL
#include "gschol.hpp"
#elif LOOKAHEAD
#include "cqr2bgslookahead.hpp"
#elif SHIFT
#include "scqr3.hpp"
#else
#include "cqr2bgs.hpp"
#endif

void conflicting_options(const boost::program_options::variables_map& vm, 
                         const char* opt1, const char* opt2)
{
    if (vm.count(opt1) && !vm[opt1].defaulted() 
        && vm.count(opt2) && !vm[opt2].defaulted())
        throw std::logic_error(std::string("Conflicting options '") 
                          + opt1 + "' and '" + opt2 + "'.\n"\
                          +"Use either block size 'b' or number of blocks 'bn'");
}


int main(int argc, char** argv) {

    //----- cli program options -----//
    using namespace boost::program_options; 
    options_description desc_commandline("Program options"); 

    desc_commandline.add_options()
        ("m", value<std::int64_t>()->default_value(10000), "Number of rows (default: 10000")
        ("n", value<std::int64_t>()->default_value(1000), "Number of columns (default: 1000")
        ("b", value<std::int64_t>()->default_value(50), "Panel size (default: 50)")
        ("bn", value<std::int64_t>(), "Number of panels")
        ("k", value<int>(), "Exponent of scientific notation of cond number of matrix")
        ("input", value<std::string>(), "Name of input bin matrix")
        ("validate,v", bool_switch()->default_value(false), "validation of the obtained Q and R factors (default: false)")
        ("help,h","Print this message");

 
    variables_map vm;
    store(parse_command_line(argc, argv, desc_commandline), vm);
    notify(vm);

    if (vm.count("help")) {
        std::cout << desc_commandline << std::endl;
        return 1;
    }

    // Matrix size
    std::int64_t m = vm["m"].as<std::int64_t>();
    std::int64_t n = vm["n"].as<std::int64_t>();
    std::int64_t block_size = vm["b"].as<std::int64_t>();

    // Panel size and the number of panels
    std::int64_t panel_size;
    if(!vm.count("b") && !vm.count("bn")) {
        std::cout << "Set either panel size 'b' or number of panels 'bn'\n";
        exit(1);
    }
    else if(vm.count("b")) {
        panel_size = vm["b"].as<std::int64_t>();
    }
    else {
        panel_size = ceil((double)n / vm["bn"].as<std::int64_t>());
    }

    const char* input_matrix_name_str = vm["input"].as<std::string>().c_str();

#ifdef LOOKAHEAD
    //same api for cpu and gpu versions
    cqr::qr2bgsloohahead algorithm(m, n, block_size);
#elif SHIFT
    cqr::qr3 algorithm(m, n);
#elif GSCHOL
    cqr::gschol algorithm(m, n, block_size);
#else 
    cqr::qr2bgs algorithm(m, n, block_size);
#endif
    algorithm.InputMatrix(input_matrix_name_str);
    algorithm.Start();
    return 0;
}
