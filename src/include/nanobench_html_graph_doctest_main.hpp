// Helpers for producing multiple comparison graphs with nanobench+doctest
// ======================================================================
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2025 CS GROUP
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// ======================================================================
//
// Authors:
// - Luc Hermitte, initial author
//
// ======================================================================

// Defines:
// - A main() function that replaces what
//   DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN would have generated to enable a
//   new parameter: "--renderto=webpage.html".
// - A global pointer variable: graph_renderer to be used if not null.

#ifndef nanobench_html_graph_doctest_main
#define nanobench_html_graph_doctest_main

#include "nanobench_html_graph_renderer.hpp"
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

// I haven't found a better way (than a global) to have this available
// to every test case
HtmlGraphRenderer * graph_renderer = nullptr;

/**
 * Helper function to render a benchmark into a new graph.
 * If initialized, the benchmark is rendered into a graph. Nothing is
 * done otherwise.
 * @tparam Strings  Extra parameters forwarded to `HtmlGraphRenderer::skeleton()`.
 * @param[in] b  nanobench micro-benchmark to render as graph.
 * @param[in] s  Extra parameters to forward to `skeleton()`
 *
 * @throw std::bad_alloc if memory is exhausted.
 * @throw AnyThing that `ankerl::nanobench::render()` may throw.
 */
template <typename... Strings>
void render_graph(ankerl::nanobench::Bench const& b, Strings const&... s)
{
    if (graph_renderer) {
        graph_renderer->render_to(b, s...);
    }
}

#ifndef NANOBENCH_VIOLIN_OPTIONS
/**
 * Initialization options for the `HtmlGraphRenderer` instance.
 * The options are meant to be passed as a series of calls to
 * `HtmlGraphRenderer` _à la_ _builder pattern_.
 *
 * For instance:
 *
 * ```c++
 * #define NANOBENCH_VIOLIN_OPTIONS \
 *      .showepochs(true) \
 *      .rangemode("tozero")
 * ```
 *
 * This macro is meant to be overriden before including this header file.
 */
#define NANOBENCH_VIOLIN_OPTIONS
#endif

DOCTEST_MSVC_SUPPRESS_WARNING_WITH_PUSH(4007) // 'function' : must be 'attribute' - see issue #182
int main(int argc, char** argv)
{
    auto ctx = doctest::Context();
    auto l_output = HtmlGraphRenderer("violin")
        .showlegend(true)
        NANOBENCH_VIOLIN_OPTIONS
        ;

    ctx.applyCommandLine(argc, argv);
    doctest::String output_filename;
    doctest::parseOption(argc, argv, DOCTEST_CONFIG_OPTIONS_PREFIX "renderto=", &output_filename, "");

    if (output_filename.size() > 0) {
        graph_renderer = &l_output;
        graph_renderer->open(output_filename.c_str());
    }

    return ctx.run();
}
DOCTEST_MSVC_SUPPRESS_WARNING_POP

#endif  // nanobench_html_graph_doctest_main
