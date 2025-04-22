// Helpers for producing multiple comparison graphs with nanobench+doctest
// ======================================================================
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2025 CSGroup-FRANCE/SopraSteriaGroup, Luc Hermitte
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
// Defines:
// - HtmlGraphRenderer meant to be used in nanobench::Bench::render()
//   It will trace several benchmarks instead of only 1 as it's done by
//   ankerl::nanobench::templates::htmlBoxplot()
//   Also an option permits to choose violin graphs instead of box
//   graphs
//
// - A main() function that replaces what
//   DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN would have generated to enable a
//   new parameter: "--renderto=webpage.html"
// - Defines  global pointer variable: graph_renderer to be used if not
//   null.
#ifndef nanobench_html_graph_renderer
#define nanobench_html_graph_renderer

#define ANKERL_NANOBENCH_IMPLEMENT
// #define ANKERL_NANOBENCH_LOG_ENABLED
#include <nanobench.h>

#include <cassert>
#include <fstream>
#include <stdexcept>

/** Helper class that builds an HTML graph rendered for the
 * microbenchmarks.
 *
 * This class helps produces a HTML file that draws
 * [Box plots](https://plotly.com/javascript/box-plots/) or
 * [violin plots](https://plotly.com/javascript/violin/) thanks to
 * plotly JavaScript API.
 *
 * It's very similar to what
 * `ankerl::nanobench::templates::htmlBoxplot()` returns.
 *
 * The differences are:
 * - Several benchmarks can add plots to the same HTML file.
 * - We can choose to display plot legends on the side. This permits to
 *   interactively select which plot we want to display.
 * - On top of _box plots_, _violin plots_ are also supported. Even when
 *   generating violin plots, the associated box plot, median, and mean
 *   will also be displayed to help detect quartiles, median...
 *   visually.
 * - Spacing between plots have to reduced.
 * - The name of each plot will also include the
 *   [Median Absolute Percentage Error](https://en.wikipedia.org/wiki/Mean_absolute_percentage_error)
 *   which is automatically computed by nanobench.
 * - Plotly version has been updated to the latest at the time (April
 *   2025).
 *
 * This class is not copiable, but moveable -- thanks to the internal
 * `std::ofstream` data member.
 */
class HtmlGraphRenderer
{
public:

    /**
     * Only constructor.
     * Initializes the instance.
     * @param[in] plot_type   "violin" or "box" -- other supported plot
     *                        types that plotly may support haven't been tested.
     * @param[in] showlegend  Tells to display [legend](https://plotly.com/javascript/legend/).
     * @throw None
     * @post The output file isn't yet opened.
     */
    explicit HtmlGraphRenderer(std::string plot_type, bool showlegend = false)
    : m_plot_type(std::move(plot_type))
    , m_showlegend(showlegend ? "true" : "false")
    {
        assert(!m_file.is_open());
    }

    /**
     * Opens the output HTML file and starts filling it.
     * Tries to open `filename`. An exception is thrown if not possible.
     * Otherwise, HTML header and `<body>` are automatically filled.
     * @param[in] filename  Output HTML filename.
     * @throw std::runtime_error is `filename` cannot be opened.
     * @post `bool(*this)` returns true if opening succeeds.
     */
    void open(std::string filename)
    {
        m_filename = std::move(filename);
        m_file.open(m_filename);
        if (!m_file) {
            throw std::runtime_error("Cannot render ouput to " + m_filename);
        }

        // clang-format off
        m_file <<
            "<!doctype html>\n"
            "<html>\n"
            "  <head>\n"
            // The version number may need to change from time to time
            "    <script src=\"https://cdn.plot.ly/plotly-3.0.1.min.js\"></script>\n"
            "  </head>\n"
            "  <body>\n"
            ;
        // clang-format on
    }

    /**
     * Destructor.
     * Closes the HTML tags and the file -- if opened.
     * @throw none
     */
    ~HtmlGraphRenderer()
    {
        if (!m_file.is_open()) return;
        assert(m_file);
        // clang-format off
        m_file <<
            "  </body>\n"
            "</html>\n"
            ;
        // clang-format on
        // std::cout << doctest::Color::Cyan <<"[nanobench]" << doctest::Color::None << " Rendered to " << m_filename << std::endl;
    }

    /** Tells whether an opened file has been associated to the
     * instance.
     */
    explicit operator bool() { return bool(m_file); }

    /** Getter to the associated file. */
    std::ostream& stream() { return m_file; }

    /**
     * Appends {{mustache}} template in the file to render the select
     * benchmark.
     * @tparam Strings  Extra parameters forwarded to `skeleton()`.
     * @param[in] b  nanobench micro-benchmark to render as graph.
     * @param[in] s  Extra parameters to forward to `skeleton()`
     *
     * @throw std::bad_alloc if memory is exhausted.
     * @throw AnyThing that `ankerl::nanobench::render()` may throw.
     *
     * @pre This function needs to be called after all the
     * micro-benchmarks have been executed.
     */
    template <typename... Strings>
    void render_to(ankerl::nanobench::Bench const& b, Strings const&... s)
    {
        render(skeleton(s...), b, stream());
    }

private:

    /**
     * Internal function that returns the mustache template for the
     * current benchmark.
     * @param[in] id         Name for the HTML `<div/>` that will be
     *                       created for the current benchmark.
     * @param[in] plot_type  "violin" or "box". Permits to override
     *                       locally the plot type set in the class
     *                       constructor.
     *
     * @return A {{mustache}} template that will be filled by
     *         `ankerl::nanobench::render()`
     * @throw std::bad_alloc if string cannot be created. Unlikely.
     *
     * @pre `id` has to be different for each benchmark.
     *
     * @see `ankerl::nanobench::render()` for a complete list of all
     * possible tags
     * https://nanobench.ankerl.com/reference.html#_CPPv4N6ankerl9nanobench6renderEPKcRK5BenchRNSt7ostreamE
     */
    std::string skeleton(std::string const& id = "mydiv", std::string const& plot_type = "") const
    {
        std::string const& type = !empty(plot_type) ? plot_type : m_plot_type;
        // clang-format off
        return "    <div id='" + id + "'>\n"
            // Force no 100% space in-between graphs
            "      <div class='plot-container plotly' style='width: 100%;'></div>\n"
            "    </div>\n"
            "    <script>\n"
            "        var data = [\n"
            "            {{#result}}{\n"
            "                name: '{{name}} (error: ' + (100*{{medianAbsolutePercentError(elapsed)}}).toFixed(2) + '%)',\n"
            "                y: [{{#measurement}}{{elapsed}}{{^-last}}, {{/last}}{{/measurement}}],\n"
            "            },\n"
            "            {{/result}}\n"
            "        ];\n"
            "        var title = '{{title}}';\n"
            "\n"
            "        data = data.map(a => Object.assign(a, { boxpoints: 'all', pointpos: 0, type: '" + type + "', box: {visible: true}, meanline: {visible: true} }));\n"
            "        var layout = { title: { text: title }, showlegend: "+m_showlegend+", yaxis: { title: 'time per unit', rangemode: 'tozero', autorange: true } };\n"
            "        Plotly.newPlot('" + id + "', data, layout, {responsive: true});\n"
            "    </script>\n"
            ;
            // clang-format on
    }

    std::string   m_plot_type;
    std::string   m_showlegend;
    std::string   m_filename;
    std::ofstream m_file;
};

#endif  // nanobench_html_graph_renderer
