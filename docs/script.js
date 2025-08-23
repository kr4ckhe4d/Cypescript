// Navigation and UI functionality
document.addEventListener('DOMContentLoaded', function() {
    // Initialize navigation
    initializeNavigation();
    
    // Initialize syntax highlighting
    if (typeof Prism !== 'undefined') {
        Prism.highlightAll();
    }
    
    // Initialize mobile menu
    initializeMobileMenu();
});

function initializeNavigation() {
    const navLinks = document.querySelectorAll('.nav-link');
    const sections = document.querySelectorAll('.content-section');
    
    navLinks.forEach(link => {
        link.addEventListener('click', function(e) {
            e.preventDefault();
            
            const targetId = this.getAttribute('href').substring(1);
            
            // Update active nav link
            navLinks.forEach(l => l.classList.remove('active'));
            this.classList.add('active');
            
            // Show target section
            sections.forEach(section => {
                section.classList.remove('active');
                if (section.id === targetId) {
                    section.classList.add('active');
                }
            });
            
            // Scroll to top of main content
            document.querySelector('.main-content').scrollTop = 0;
            
            // Update URL hash
            window.history.pushState(null, null, `#${targetId}`);
        });
    });
    
    // Handle initial hash
    const hash = window.location.hash.substring(1);
    if (hash) {
        const targetLink = document.querySelector(`[href="#${hash}"]`);
        if (targetLink) {
            targetLink.click();
        }
    }
}

function initializeMobileMenu() {
    // Add mobile menu toggle button
    const sidebar = document.querySelector('.sidebar');
    const mainContent = document.querySelector('.main-content');
    
    // Create mobile menu button
    const menuButton = document.createElement('button');
    menuButton.className = 'mobile-menu-btn';
    menuButton.innerHTML = '<i class="fas fa-bars"></i>';
    menuButton.style.cssText = `
        display: none;
        position: fixed;
        top: 1rem;
        left: 1rem;
        z-index: 1001;
        background: #667eea;
        color: white;
        border: none;
        padding: 0.75rem;
        border-radius: 4px;
        cursor: pointer;
        font-size: 1.2rem;
    `;
    
    document.body.appendChild(menuButton);
    
    // Show menu button on mobile
    function checkMobile() {
        if (window.innerWidth <= 768) {
            menuButton.style.display = 'block';
        } else {
            menuButton.style.display = 'none';
            sidebar.classList.remove('open');
        }
    }
    
    checkMobile();
    window.addEventListener('resize', checkMobile);
    
    // Toggle sidebar
    menuButton.addEventListener('click', function() {
        sidebar.classList.toggle('open');
    });
    
    // Close sidebar when clicking outside
    mainContent.addEventListener('click', function() {
        if (window.innerWidth <= 768) {
            sidebar.classList.remove('open');
        }
    });
}

// Code execution functions
function runExample(exampleId) {
    const codeElement = document.getElementById(exampleId);
    const outputElement = document.getElementById(exampleId + '-output');
    
    if (!codeElement || !outputElement) {
        console.error('Example elements not found:', exampleId);
        return;
    }
    
    const code = codeElement.textContent;
    
    // Show loading state
    outputElement.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Running...';
    outputElement.className = 'output';
    
    // Simulate execution delay for better UX
    setTimeout(() => {
        try {
            const result = executeCypescriptCode(code);
            outputElement.textContent = result;
            outputElement.classList.add('success');
        } catch (error) {
            outputElement.textContent = `Error: ${error.message}`;
            outputElement.classList.add('error');
        }
    }, 500);
}

function runPlayground() {
    const codeTextarea = document.getElementById('playground-code');
    const outputElement = document.getElementById('playground-output');
    
    const code = codeTextarea.value;
    
    // Show loading state
    outputElement.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Running...';
    outputElement.className = 'output';
    
    setTimeout(() => {
        try {
            const result = executeCypescriptCode(code);
            outputElement.textContent = result;
            outputElement.classList.add('success');
        } catch (error) {
            outputElement.textContent = `Error: ${error.message}`;
            outputElement.classList.add('error');
        }
    }, 500);
}

function clearPlayground() {
    const codeTextarea = document.getElementById('playground-code');
    const outputElement = document.getElementById('playground-output');
    
    codeTextarea.value = `// Write your Cypescript code here...
println("Hello from the playground!");

// Arrays
let numbers: i32[] = [1, 2, 3, 4, 5];
print("Array: ");
println(numbers);
print("First element: ");
println(numbers[0]);

// Objects
let person = { name: "Alice", age: 25 };
print("Name: ");
println(person.name);
print("Age: ");
println(person.age);`;
    outputElement.textContent = '';
    outputElement.className = 'output';
}

// Smooth scrolling for anchor links
document.addEventListener('click', function(e) {
    if (e.target.tagName === 'A' && e.target.getAttribute('href').startsWith('#')) {
        e.preventDefault();
        const targetId = e.target.getAttribute('href').substring(1);
        const targetElement = document.getElementById(targetId);
        
        if (targetElement) {
            targetElement.scrollIntoView({
                behavior: 'smooth',
                block: 'start'
            });
        }
    }
});

// Copy code functionality
function addCopyButtons() {
    const codeBlocks = document.querySelectorAll('.code-example pre');
    
    codeBlocks.forEach(block => {
        const copyButton = document.createElement('button');
        copyButton.className = 'copy-btn';
        copyButton.innerHTML = '<i class="fas fa-copy"></i>';
        copyButton.title = 'Copy code';
        
        copyButton.style.cssText = `
            position: absolute;
            top: 0.5rem;
            right: 0.5rem;
            background: rgba(255, 255, 255, 0.1);
            color: white;
            border: none;
            padding: 0.5rem;
            border-radius: 4px;
            cursor: pointer;
            opacity: 0;
            transition: opacity 0.3s ease;
        `;
        
        const codeExample = block.closest('.code-example');
        codeExample.style.position = 'relative';
        codeExample.appendChild(copyButton);
        
        // Show copy button on hover
        codeExample.addEventListener('mouseenter', () => {
            copyButton.style.opacity = '1';
        });
        
        codeExample.addEventListener('mouseleave', () => {
            copyButton.style.opacity = '0';
        });
        
        // Copy functionality
        copyButton.addEventListener('click', async () => {
            const code = block.querySelector('code').textContent;
            
            try {
                await navigator.clipboard.writeText(code);
                copyButton.innerHTML = '<i class="fas fa-check"></i>';
                setTimeout(() => {
                    copyButton.innerHTML = '<i class="fas fa-copy"></i>';
                }, 2000);
            } catch (err) {
                console.error('Failed to copy code:', err);
            }
        });
    });
}

// Initialize copy buttons after DOM is loaded
document.addEventListener('DOMContentLoaded', addCopyButtons);

// Search functionality (basic)
function initializeSearch() {
    const searchInput = document.createElement('input');
    searchInput.type = 'text';
    searchInput.placeholder = 'Search documentation...';
    searchInput.className = 'search-input';
    
    searchInput.style.cssText = `
        width: 100%;
        padding: 0.75rem;
        margin: 1rem 1.5rem;
        border: 1px solid rgba(255, 255, 255, 0.2);
        border-radius: 4px;
        background: rgba(255, 255, 255, 0.1);
        color: white;
        font-size: 0.9rem;
    `;
    
    searchInput.addEventListener('input', function(e) {
        const query = e.target.value.toLowerCase();
        const navLinks = document.querySelectorAll('.nav-link');
        
        navLinks.forEach(link => {
            const text = link.textContent.toLowerCase();
            const listItem = link.parentElement;
            
            if (text.includes(query) || query === '') {
                listItem.style.display = 'block';
            } else {
                listItem.style.display = 'none';
            }
        });
    });
    
    // Insert search input after logo
    const logo = document.querySelector('.logo');
    logo.after(searchInput);
}

// Initialize search
document.addEventListener('DOMContentLoaded', initializeSearch);

// Keyboard shortcuts
document.addEventListener('keydown', function(e) {
    // Ctrl/Cmd + K to focus search
    if ((e.ctrlKey || e.metaKey) && e.key === 'k') {
        e.preventDefault();
        const searchInput = document.querySelector('.search-input');
        if (searchInput) {
            searchInput.focus();
        }
    }
    
    // Escape to clear search
    if (e.key === 'Escape') {
        const searchInput = document.querySelector('.search-input');
        if (searchInput && document.activeElement === searchInput) {
            searchInput.value = '';
            searchInput.dispatchEvent(new Event('input'));
            searchInput.blur();
        }
    }
});

// Theme toggle (future enhancement)
function initializeThemeToggle() {
    const themeToggle = document.createElement('button');
    themeToggle.className = 'theme-toggle';
    themeToggle.innerHTML = '<i class="fas fa-moon"></i>';
    themeToggle.title = 'Toggle dark/light theme';
    
    // Add to sidebar footer
    const sidebarFooter = document.querySelector('.sidebar-footer');
    sidebarFooter.appendChild(themeToggle);
    
    // Theme toggle functionality would go here
    themeToggle.addEventListener('click', function() {
        // Future: implement theme switching
        console.log('Theme toggle clicked - feature coming soon!');
    });
}

// Performance monitoring
function trackPageViews() {
    const navLinks = document.querySelectorAll('.nav-link');
    
    navLinks.forEach(link => {
        link.addEventListener('click', function() {
            const section = this.getAttribute('href').substring(1);
            console.log(`Viewed section: ${section}`);
            
            // Future: send analytics data
            // analytics.track('section_viewed', { section });
        });
    });
}

// Initialize performance tracking
document.addEventListener('DOMContentLoaded', trackPageViews);
