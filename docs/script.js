// Cypescript Docs — MDN-style site behavior:
// hash routing, sidebar state, breadcrumbs, per-article TOC, search, copy
// buttons, and the playground (backed by cypescript-interpreter.js).

(function () {
  'use strict';

  const articles = Array.from(document.querySelectorAll('.doc-article'));
  const sidebarLinks = Array.from(document.querySelectorAll('.sidebar a[href^="#"]'));
  const breadcrumbsEl = document.getElementById('breadcrumbs');
  const tocEl = document.getElementById('toc-list');

  // ---------- Routing ----------
  function currentId() {
    const hash = window.location.hash.replace('#', '');
    return document.getElementById(hash) ? hash : 'landing';
  }

  function show(id) {
    const article = document.getElementById(id) || document.getElementById('landing');

    articles.forEach(a => a.classList.toggle('active', a === article));
    sidebarLinks.forEach(l =>
      l.classList.toggle('active', l.getAttribute('href') === '#' + article.id));

    renderBreadcrumbs(article);
    renderToc(article);
    document.title = (article.dataset.title || 'Cypescript') + ' — Cypescript Docs';
    window.scrollTo(0, 0);
  }

  function renderBreadcrumbs(article) {
    const crumbs = (article.dataset.crumbs || '').split('>').map(s => s.trim()).filter(Boolean);
    let html = '<a href="#landing">Cypescript</a>';
    crumbs.forEach(c => { html += '<span class="crumb-sep">›</span>' + c; });
    if (article.id !== 'landing') {
      html += '<span class="crumb-sep">›</span><strong>' + (article.dataset.title || '') + '</strong>';
    }
    breadcrumbsEl.innerHTML = html;
  }

  function renderToc(article) {
    const headings = Array.from(article.querySelectorAll('h2'));
    tocEl.innerHTML = '';
    headings.forEach((h, i) => {
      if (!h.id) h.id = article.id + '-h' + i;
      const li = document.createElement('li');
      const a = document.createElement('a');
      a.href = '#' + article.id;          // stay on the article...
      a.textContent = h.textContent;
      a.addEventListener('click', e => {  // ...but scroll to the heading
        e.preventDefault();
        h.scrollIntoView({ behavior: 'smooth', block: 'start' });
      });
      li.appendChild(a);
      tocEl.appendChild(li);
    });
    document.getElementById('toc').style.visibility = headings.length ? 'visible' : 'hidden';
  }

  window.addEventListener('hashchange', () => show(currentId()));

  // ---------- Search ----------
  const searchInput = document.getElementById('search-input');
  const searchResults = document.getElementById('search-results');

  const searchIndex = articles
    .filter(a => a.id !== 'landing')
    .map(a => ({
      id: a.id,
      title: a.dataset.title || a.id,
      crumbs: a.dataset.crumbs || '',
      keywords: (a.dataset.keywords || '').toLowerCase(),
      text: a.textContent.toLowerCase(),
    }));

  function runSearch(query) {
    const q = query.trim().toLowerCase();
    searchResults.innerHTML = '';
    if (q.length < 2) { searchResults.classList.remove('open'); return; }

    const hits = searchIndex
      .map(entry => {
        let score = 0;
        if (entry.title.toLowerCase() === q) score += 100;
        else if (entry.title.toLowerCase().includes(q)) score += 50;
        if (entry.keywords.includes(q)) score += 60;
        // weight by how often the term appears in the article body
        score += Math.min(entry.text.split(q).length - 1, 20);
        return { entry, score };
      })
      .filter(h => h.score > 0)
      .sort((a, b) => b.score - a.score)
      .slice(0, 8);

    hits.forEach(({ entry }) => {
      const a = document.createElement('a');
      a.href = '#' + entry.id;
      a.innerHTML = entry.title + '<small>' + entry.crumbs + '</small>';
      a.addEventListener('click', () => {
        searchResults.classList.remove('open');
        searchInput.value = '';
      });
      searchResults.appendChild(a);
    });
    searchResults.classList.toggle('open', hits.length > 0);
  }

  searchInput.addEventListener('input', () => runSearch(searchInput.value));
  searchInput.addEventListener('keydown', e => {
    if (e.key === 'Enter') {
      const first = searchResults.querySelector('a');
      if (first) first.click();
    }
    if (e.key === 'Escape') searchResults.classList.remove('open');
  });
  document.addEventListener('click', e => {
    if (!e.target.closest('.header-search')) searchResults.classList.remove('open');
  });
  document.addEventListener('keydown', e => {
    if (e.key === '/' && document.activeElement !== searchInput &&
        document.activeElement.tagName !== 'TEXTAREA') {
      e.preventDefault();
      searchInput.focus();
    }
  });

  // ---------- Copy buttons ----------
  document.querySelectorAll('.code-example pre').forEach(pre => {
    const header = pre.previousElementSibling;
    if (!header || !header.classList.contains('example-header')) return;
    const btn = document.createElement('button');
    btn.className = 'copy-btn';
    btn.textContent = 'Copy';
    btn.addEventListener('click', () => {
      navigator.clipboard.writeText(pre.textContent).then(() => {
        btn.textContent = 'Copied!';
        setTimeout(() => (btn.textContent = 'Copy'), 1500);
      });
    });
    header.appendChild(btn);
  });

  // ---------- Playground ----------
  const runBtn = document.getElementById('playground-run');
  if (runBtn) {
    const editor = document.getElementById('playground-code');
    const output = document.getElementById('playground-output');
    const outputPre = output.querySelector('pre');

    runBtn.addEventListener('click', () => {
      try {
        const result = executeCypescriptCode(editor.value);
        outputPre.textContent = result || '(no output)';
        outputPre.style.color = '';
      } catch (err) {
        outputPre.textContent = err.message;
        outputPre.style.color = '#d30038';
      }
    });

    document.getElementById('playground-clear').addEventListener('click', () => {
      editor.value = '';
      outputPre.textContent = '';
    });
  }

  // Initial render
  show(currentId());
})();
