# Cypescript Web Documentation

MDN-style documentation site for Cypescript: categorized sidebar, per-feature
reference pages (Syntax → Description → Examples → See also), full-text search,
breadcrumbs, an "In this article" TOC, light/dark themes, and an interactive
playground for the core language subset.

## Run locally

```bash
./launch-docs.sh          # from the repo root
# or:
python3 -m http.server 8080 --directory docs
```

## Structure

| File | Purpose |
|---|---|
| `index.html` | All content: 24 articles (guides + reference + playground) as `<article class="doc-article">` sections |
| `styles.css` | MDN-style theme (CSS variables, automatic dark mode) |
| `script.js` | Hash routing, sidebar state, breadcrumbs, TOC, search, copy buttons, playground glue |
| `cypescript-interpreter.js` | In-browser interpreter backing the playground (core subset only) |
| `serve.py` | Simple local server used by `launch-docs.sh` |

## Adding an article

1. Add an `<article id="ref-thing" class="doc-article" data-title="thing"
   data-crumbs="Reference &gt; Statements" data-keywords="synonyms here">`
   before the playground article, following the MDN anatomy:
   summary → `Syntax` (a `pre.syntaxbox`) → parameter `<dl>` → `Description`
   → `Examples` → `See also`.
2. Add a sidebar link `<a href="#ref-thing">`.
3. Escape `<` as `&lt;` inside code blocks (generics and comparisons!).

Routing, breadcrumbs, the TOC, and search pick the new article up automatically.
