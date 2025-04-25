// Helpers for producing multiple comparison graphs with nanobench+doctest
// ======================================================================
// Copyright (c) 2025 CS GROUP
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
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
//
// Defines:
// - HtmlGraphRenderer meant to be used in nanobench::Bench::render()
//   It will trace several benchmarks instead of only 1 as it's done by
//   ankerl::nanobench::templates::htmlBoxplot()
//   Also an option permits to choose violin graphs instead of box
//   graphs

#ifndef nanobench_html_graph_renderer
#define nanobench_html_graph_renderer

#define ANKERL_NANOBENCH_IMPLEMENT
// #define ANKERL_NANOBENCH_LOG_ENABLED
#include <nanobench.h>

#include <cassert>
#include <fstream>
#include <stdexcept>
#include <string>

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
    explicit HtmlGraphRenderer(std::string plot_type)
    : m_plot_type(std::move(plot_type))
    {
        assert(!m_file.is_open());
    }

    HtmlGraphRenderer(HtmlGraphRenderer const&) = delete;
    HtmlGraphRenderer(HtmlGraphRenderer     &&) = default;
    HtmlGraphRenderer& operator=(HtmlGraphRenderer const&) = delete;
    HtmlGraphRenderer& operator=(HtmlGraphRenderer     &&) = default;

#if defined(__cpp_explicit_this_parameter)
    // requires C++23, and g++14 or clang++20 (even if it should work with clang++18)
    /**
     * Tells to display plot legend.
     * Setter meant to be used from _builder pattern_.
     * It works on lvalue and rvalue instances of `HtmlGraphRenderer`
     * @param[in] do_show  Shall we display the legend?
     * @return this
     * @see https://plotly.com/javascript/legend/
     */
    template <typename Self>
    Self&& showlegend(this Self&& self, bool do_show)
    {
        self.m_show_legend = do_show ? "true" : "false";
        return std::forward<Self>(self);
    }

    /**
     * Tells to display the actual number of epochs in plot titles.
     *
     * Setter meant to be used from _builder pattern_.
     * It works on lvalue and rvalue instances of `HtmlGraphRenderer`.
     * @param[in] do_show  Shall we report the number of epochs?
     * @return this
     * @see nanobench definition of epoch
     * https://nanobench.ankerl.com/reference.html#_CPPv4N6ankerl9nanobench5Bench6epochsE6size_t
     */
    template <typename Self>
    Self&& showepochs(this Self&& self, bool do_show)
    {
        self.m_show_epochs  = do_show ? "; epochs: {{epochs}}" : "";
        return std::forward<Self>(self);
    }

    /**
     * Sets the rangemode option to use.
     *
     * Setter meant to be used from _builder pattern_.
     * It works on lvalue and rvalue instances of `HtmlGraphRenderer`.
     * @param[in] mode Range mode name to use.
     *                 If empty, the option in not set in the plot.
     *                 Set to "tozero" by default.
     * @return this
     * @see https://plotly.com/javascript/reference/layout/yaxis/#layout-yaxis-rangemode
     */
    template <typename Self>
    Self&& rangemode(this Self&& self, std::string const& mode)
    {
        self.m_range_mode = !empty(mode) ? ", rangemode: '" + mode + "'" : "";
        return std::forward<Self>(self);
    }

#else
    // Same setters, but overloaded for lvalues and rvalues, before C++23 "explicit this parameter"
    // feature.
    HtmlGraphRenderer&& showlegend(bool do_show) &&
    {
        m_show_legend = do_show ? "true" : "false";
        return std::move(*this);
    }
    HtmlGraphRenderer& showlegend(bool do_show) &
    {
        m_show_legend = do_show ? "true" : "false";
        return *this;
    }

    HtmlGraphRenderer&& showepochs(bool do_show) &&
    {
        m_show_epochs  = do_show ? "; epochs: {{epochs}}" : "";
        return std::move(*this);
    }
    HtmlGraphRenderer& showepochs(bool do_show) &
    {
        m_show_epochs  = do_show ? "; epochs: {{epochs}}" : "";
        return *this;
    }

    HtmlGraphRenderer&& rangemode(std::string const& mode) &&
    {
        m_range_mode = !empty(mode) ? ", rangemode: '" + mode + "'" : "";
        return std::move(*this);
    }
    HtmlGraphRenderer& rangemode(std::string const& mode) &
    {
        m_range_mode = !empty(mode) ? ", rangemode: '" + mode + "'" : "";
        return *this;
    }
#endif

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
    [[nodiscard]]
    explicit operator bool()
    {
        return bool(m_file);
    }

    /** Getter to the associated file. */
    [[nodiscard]]
    std::ostream& stream()
    {
        return m_file;
    }

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
    [[nodiscard]]
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
            "                name: '{{name}} (error: ' + (100*{{medianAbsolutePercentError(elapsed)}}).toFixed(2) + '%" + m_show_epochs + ")',\n"
            "                y: [{{#measurement}}{{elapsed}}{{^-last}}, {{/last}}{{/measurement}}],\n"
            "            },\n"
            "            {{/result}}\n"
            "        ];\n"
            "        var title = '{{title}}';\n"
            "\n"
            "        data = data.map(a => Object.assign(a, { boxpoints: 'all', pointpos: 0, type: '" + type + "', box: {visible: true}, meanline: {visible: true} }));\n"
            "        var layout = { title: { text: title }, showlegend: "+m_show_legend+", yaxis: { title: 'time per unit'" + m_range_mode + ", autorange: true } };\n"
            "        Plotly.newPlot('" + id + "', data, layout, {responsive: true});\n"
            "    </script>\n"
            ;
            // clang-format on
    }

    std::string   m_plot_type;
    std::string   m_show_legend = "false";
    std::string   m_show_epochs = "";
    std::string   m_range_mode  =  ", rangemode: 'tozero'";
    std::string   m_filename;
    std::ofstream m_file;
};

#endif  // nanobench_html_graph_renderer
