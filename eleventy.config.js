const fs = require("node:fs");
const path = require("node:path");

function escapeHtml(value) {
  return value
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#39;");
}

module.exports = function (eleventyConfig) {
  eleventyConfig.addPassthroughCopy({ "www/static": "." });
  eleventyConfig.addWatchTarget("src");

  eleventyConfig.addShortcode("codeSnippet", (sourcePath) => {
    const fullPath = path.resolve(process.cwd(), sourcePath);
    try {
      const contents = fs.readFileSync(fullPath, "utf8");
      return `<pre><code class="language-c">${escapeHtml(contents)}</code></pre>`;
    } catch (err) {
      console.warn(`Unable to load snippet for ${sourcePath}:`, err);
      return `<pre><code class="language-text">Failed to load ${escapeHtml(sourcePath)}</code></pre>`;
    }
  });

  return {
    dir: {
      input: "www",
      includes: "_includes",
      data: "_data",
      output: "public",
    },
    htmlTemplateEngine: "njk",
    markdownTemplateEngine: "njk",
    templateFormats: ["njk"],
  };
};
