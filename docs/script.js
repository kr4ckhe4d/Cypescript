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

// Initialize performance charts
document.addEventListener('DOMContentLoaded', function() {
    initializePerformanceCharts();
});

// Performance Charts Initialization
function initializePerformanceCharts() {
    // Chart.js default configuration
    Chart.defaults.font.family = "'Segoe UI', Tahoma, Geneva, Verdana, sans-serif";
    Chart.defaults.font.size = 12;
    
    // 1. Multi-Language Performance Comparison Chart
    const multiLanguageCtx = document.getElementById('multiLanguageChart');
    if (multiLanguageCtx) {
        new Chart(multiLanguageCtx, {
            type: 'bar',
            data: {
                labels: ['Array Processing', 'Complex Objects', 'Intensive Computation'],
                datasets: [{
                    label: 'JavaScript',
                    data: [98, 0.57, 723],
                    backgroundColor: 'rgba(243, 156, 18, 0.8)',
                    borderColor: 'rgba(243, 156, 18, 1)',
                    borderWidth: 2
                }, {
                    label: 'Cypescript (Memory-Opt ✅)',
                    data: [301, 466, 986],
                    backgroundColor: 'rgba(39, 174, 96, 0.8)',
                    borderColor: 'rgba(39, 174, 96, 1)',
                    borderWidth: 2
                }, {
                    label: 'Cypescript (Basic)',
                    data: [438, null, null], // Only array processing tested
                    backgroundColor: 'rgba(231, 76, 60, 0.8)',
                    borderColor: 'rgba(231, 76, 60, 1)',
                    borderWidth: 2
                }, {
                    label: 'Python',
                    data: [644, 9.20, 94666],
                    backgroundColor: 'rgba(52, 152, 219, 0.8)',
                    borderColor: 'rgba(52, 152, 219, 1)',
                    borderWidth: 2
                }]
            },
            options: {
                responsive: true,
                plugins: {
                    title: {
                        display: true,
                        text: 'Multi-Language Performance: Complex Objects + Memory Optimization!',
                        font: { size: 16, weight: 'bold' },
                        color: 'white'
                    },
                    legend: {
                        labels: { color: 'white' }
                    },
                    tooltip: {
                        callbacks: {
                            afterLabel: function(context) {
                                if (context.label === 'Complex Objects') {
                                    if (context.dataset.label === 'JavaScript') {
                                        return '250K property accesses';
                                    } else if (context.dataset.label.includes('Cypescript')) {
                                        return '2.5M property accesses (10x scale!)';
                                    } else if (context.dataset.label === 'Python') {
                                        return '250K property accesses';
                                    }
                                }
                                return '';
                            }
                        }
                    }
                },
                scales: {
                    y: {
                        type: 'logarithmic',
                        beginAtZero: true,
                        title: {
                            display: true,
                            text: 'Execution Time (ms) - Log Scale',
                            color: 'white'
                        },
                        ticks: { 
                            color: 'white',
                            callback: function(value) {
                                if (value >= 1000) {
                                    return (value / 1000).toFixed(0) + 'k';
                                }
                                return value;
                            }
                        },
                        grid: { color: 'rgba(255, 255, 255, 0.2)' }
                    },
                    x: {
                        ticks: { color: 'white' },
                        grid: { color: 'rgba(255, 255, 255, 0.2)' }
                    }
                }
            }
        });
    }
    
    // 2. Performance Scaling Chart
    const scalingCtx = document.getElementById('scalingChart');
    if (scalingCtx) {
        new Chart(scalingCtx, {
            type: 'line',
            data: {
                labels: ['100 elements', '10K elements', '100K elements', '1M elements'],
                datasets: [{
                    label: 'JavaScript',
                    data: [80, 800, 8000, 80000],
                    borderColor: 'rgba(243, 156, 18, 1)',
                    backgroundColor: 'rgba(243, 156, 18, 0.2)',
                    borderWidth: 3,
                    fill: true
                }, {
                    label: 'Cypescript (Current)',
                    data: [481, 600, 4000, 25000],
                    borderColor: 'rgba(231, 76, 60, 1)',
                    backgroundColor: 'rgba(231, 76, 60, 0.2)',
                    borderWidth: 3,
                    fill: true
                }]
            },
            options: {
                responsive: true,
                plugins: {
                    title: {
                        display: true,
                        text: 'Performance Scaling by Workload Size',
                        font: { size: 16, weight: 'bold' }
                    }
                },
                scales: {
                    y: {
                        type: 'logarithmic',
                        beginAtZero: true,
                        title: {
                            display: true,
                            text: 'Execution Time (ms) - Log Scale'
                        }
                    },
                    x: {
                        title: {
                            display: true,
                            text: 'Dataset Size'
                        }
                    }
                }
            }
        });
    }
    
    // 3. Optimization Impact Chart
    const optimizationCtx = document.getElementById('optimizationChart');
    if (optimizationCtx) {
        new Chart(optimizationCtx, {
            type: 'bar',
            data: {
                labels: ['Baseline', 'Basic Opt', 'Advanced', 'Memory Opt ✅', 'NEON SIMD ✅', 'Target'],
                datasets: [{
                    label: 'Small Workloads (ms)',
                    data: [507, 438, 302, 301, 45, 25],
                    backgroundColor: [
                        'rgba(231, 76, 60, 0.8)',   // Baseline
                        'rgba(52, 152, 219, 0.8)',  // Basic
                        'rgba(46, 204, 113, 0.8)',  // Advanced
                        'rgba(39, 174, 96, 0.8)',   // Memory - implemented
                        'rgba(155, 89, 182, 0.8)',  // NEON - implemented
                        'rgba(241, 196, 15, 0.8)'   // Target
                    ],
                    borderWidth: 2
                }]
            },
            options: {
                responsive: true,
                plugins: {
                    title: {
                        display: true,
                        text: 'Optimization Impact: Memory Optimization Implemented!',
                        font: { size: 16, weight: 'bold' }
                    },
                    legend: { display: false }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        title: {
                            display: true,
                            text: 'Execution Time (ms)'
                        }
                    }
                }
            }
        });
    }
    
    // 4. C++ Integration Performance Chart
    const cppCtx = document.getElementById('cppIntegrationChart');
    if (cppCtx) {
        new Chart(cppCtx, {
            type: 'doughnut',
            data: {
                labels: ['Pure Cypescript', 'Cypescript + C++'],
                datasets: [{
                    data: [481, 374],
                    backgroundColor: [
                        'rgba(231, 76, 60, 0.8)',
                        'rgba(46, 204, 113, 0.8)'
                    ],
                    borderWidth: 3,
                    borderColor: 'white'
                }]
            },
            options: {
                responsive: true,
                plugins: {
                    title: {
                        display: true,
                        text: 'C++ Integration Performance Boost',
                        font: { size: 16, weight: 'bold' },
                        color: 'white'
                    },
                    legend: {
                        labels: { color: 'white' }
                    }
                }
            }
        });
    }
    
    // 5. Small Workload Future Performance Chart
    const smallWorkloadCtx = document.getElementById('smallWorkloadChart');
    if (smallWorkloadCtx) {
        new Chart(smallWorkloadCtx, {
            type: 'bar',
            data: {
                labels: ['JavaScript', 'Cypescript (Advanced ✅)', 'Cypescript (w/ PGO)', 'Cypescript (Target)', 'Python'],
                datasets: [{
                    label: 'Execution Time (ms)',
                    data: [98, 323, 270, 25, 644],
                    backgroundColor: [
                        'rgba(243, 156, 18, 0.8)',  // JavaScript
                        'rgba(39, 174, 96, 0.8)',   // Advanced - achieved
                        'rgba(52, 152, 219, 0.8)',  // PGO - realistic
                        'rgba(46, 204, 113, 0.8)',  // Target
                        'rgba(155, 89, 182, 0.8)'   // Python
                    ],
                    borderWidth: 2
                }]
            },
            options: {
                responsive: true,
                plugins: {
                    title: {
                        display: true,
                        text: 'Multi-Language Performance Context',
                        font: { size: 16, weight: 'bold' }
                    },
                    legend: { display: false }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        title: {
                            display: true,
                            text: 'Time (ms)'
                        }
                    }
                }
            }
        });
    }
    
    // 6. Large Workload Future Performance Chart
    const largeWorkloadCtx = document.getElementById('largeWorkloadChart');
    if (largeWorkloadCtx) {
        new Chart(largeWorkloadCtx, {
            type: 'bar',
            data: {
                labels: ['JavaScript', 'Cypescript (Current)', 'Cypescript (Future)'],
                datasets: [{
                    label: 'Execution Time (seconds)',
                    data: [80, 25, 2],
                    backgroundColor: [
                        'rgba(243, 156, 18, 0.8)',
                        'rgba(231, 76, 60, 0.8)',
                        'rgba(46, 204, 113, 0.8)'
                    ],
                    borderWidth: 2
                }]
            },
            options: {
                responsive: true,
                plugins: {
                    legend: { display: false }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        title: {
                            display: true,
                            text: 'Time (seconds)'
                        }
                    }
                }
            }
        });
    }
}

// Performance Demo functionality
document.addEventListener('DOMContentLoaded', function() {
    const performanceButton = document.getElementById('run-performance-test');
    const performanceResults = document.getElementById('performance-results');
    const performanceOutput = document.getElementById('performance-output');
    
    if (performanceButton) {
        performanceButton.addEventListener('click', runPerformanceDemo);
    }
    
    function runPerformanceDemo() {
        performanceButton.disabled = true;
        performanceButton.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Running...';
        
        // Show results container
        performanceResults.style.display = 'block';
        performanceOutput.innerHTML = '<p>Running performance tests...</p>';
        
        // Run tests after a short delay to show loading state
        setTimeout(() => {
            const results = runBrowserBenchmarks();
            displayPerformanceResults(results);
            
            performanceButton.disabled = false;
            performanceButton.innerHTML = '<i class="fas fa-play"></i> Run Performance Test';
        }, 100);
    }
    
    function runBrowserBenchmarks() {
        const results = {};
        
        // Test 1: Array Sum Performance
        console.log('Running array sum benchmark...');
        const arraySize = 1000;
        const iterations = 10000;
        const testArray = Array.from({length: arraySize}, (_, i) => i + 1);
        
        const sumStart = performance.now();
        let totalSum = 0;
        for (let iter = 0; iter < iterations; iter++) {
            let currentSum = 0;
            for (let i = 0; i < arraySize; i++) {
                currentSum += testArray[i];
            }
            totalSum += currentSum;
        }
        const sumEnd = performance.now();
        results.arraySum = {
            time: sumEnd - sumStart,
            operations: iterations * arraySize,
            result: totalSum
        };
        
        // Test 2: Prime Number Generation
        console.log('Running prime generation benchmark...');
        const primeLimit = 1000;
        const primeStart = performance.now();
        let primeCount = 0;
        
        for (let num = 2; num <= primeLimit; num++) {
            let isPrime = true;
            for (let divisor = 2; divisor * divisor <= num; divisor++) {
                if (num % divisor === 0) {
                    isPrime = false;
                    break;
                }
            }
            if (isPrime) primeCount++;
        }
        
        const primeEnd = performance.now();
        results.primeGeneration = {
            time: primeEnd - primeStart,
            primesFound: primeCount,
            limit: primeLimit
        };
        
        // Test 3: Fibonacci Sequence
        console.log('Running Fibonacci benchmark...');
        const fibIterations = 1000;
        const fibLimit = 20;
        const fibStart = performance.now();
        let fibTotal = 0;
        
        for (let iter = 0; iter < fibIterations; iter++) {
            for (let n = 0; n <= fibLimit; n++) {
                let a = 0, b = 1, fib = 0;
                if (n === 0) fib = 0;
                else if (n === 1) fib = 1;
                else {
                    for (let i = 2; i <= n; i++) {
                        fib = a + b;
                        a = b;
                        b = fib;
                    }
                }
                fibTotal += fib;
            }
        }
        
        const fibEnd = performance.now();
        results.fibonacci = {
            time: fibEnd - fibStart,
            iterations: fibIterations,
            total: fibTotal
        };
        
        // Test 4: Array Sorting
        console.log('Running sorting benchmark...');
        const sortIterations = 100;
        const sortArray = [64, 34, 25, 12, 22, 11, 90, 88, 76, 50, 42, 30, 18, 95, 70];
        const sortStart = performance.now();
        let sortOperations = 0;
        
        for (let iter = 0; iter < sortIterations; iter++) {
            const arr = [...sortArray]; // Copy array
            // Bubble sort
            for (let i = 0; i < arr.length - 1; i++) {
                for (let j = 0; j < arr.length - i - 1; j++) {
                    if (arr[j] > arr[j + 1]) {
                        [arr[j], arr[j + 1]] = [arr[j + 1], arr[j]];
                        sortOperations++;
                    }
                }
            }
        }
        
        const sortEnd = performance.now();
        results.sorting = {
            time: sortEnd - sortStart,
            iterations: sortIterations,
            operations: sortOperations
        };
        
        return results;
    }
    
    function displayPerformanceResults(results) {
        const totalTime = results.arraySum.time + results.primeGeneration.time + 
                         results.fibonacci.time + results.sorting.time;
        
        const html = `
            <div class="performance-summary">
                <h4>🚀 Browser JavaScript Performance Results</h4>
                <div class="result-item">
                    <strong>Array Sum (${results.arraySum.operations.toLocaleString()} operations):</strong>
                    <span class="time">${results.arraySum.time.toFixed(2)}ms</span>
                </div>
                <div class="result-item">
                    <strong>Prime Generation (up to ${results.primeGeneration.limit}):</strong>
                    <span class="time">${results.primeGeneration.time.toFixed(2)}ms</span>
                    <span class="detail">(Found ${results.primeGeneration.primesFound} primes)</span>
                </div>
                <div class="result-item">
                    <strong>Fibonacci Sequence (${results.fibonacci.iterations} iterations):</strong>
                    <span class="time">${results.fibonacci.time.toFixed(2)}ms</span>
                </div>
                <div class="result-item">
                    <strong>Array Sorting (${results.sorting.iterations} iterations):</strong>
                    <span class="time">${results.sorting.time.toFixed(2)}ms</span>
                    <span class="detail">(${results.sorting.operations} swaps)</span>
                </div>
                <div class="total-time">
                    <strong>Total Time: ${totalTime.toFixed(2)}ms</strong>
                </div>
                <div class="comparison-note">
                    <p><strong>💡 Comparison with Benchmarks:</strong></p>
                    <p>This browser test shows JavaScript's excellent performance on smaller datasets. 
                    The native Cypescript advantage becomes apparent with larger, more intensive computations.</p>
                    <p><em>Your browser: ${navigator.userAgent.includes('Chrome') ? 'Chrome V8' : 
                                        navigator.userAgent.includes('Firefox') ? 'Firefox SpiderMonkey' : 
                                        navigator.userAgent.includes('Safari') ? 'Safari JavaScriptCore' : 'Unknown'}</em></p>
                </div>
            </div>
        `;
        
        performanceOutput.innerHTML = html;
        
        // Create comparison chart
        createBrowserComparisonChart(results, totalTime);
    }
    
    function createBrowserComparisonChart(results, browserTime) {
        const chartCtx = document.getElementById('browserComparisonChart');
        if (chartCtx) {
            new Chart(chartCtx, {
                type: 'bar',
                data: {
                    labels: ['Your Browser', 'Cypescript (Benchmark)', 'JavaScript (Benchmark)'],
                    datasets: [{
                        label: 'Execution Time (ms)',
                        data: [browserTime, 481, 80],
                        backgroundColor: [
                            'rgba(52, 152, 219, 0.8)',
                            'rgba(231, 76, 60, 0.8)',
                            'rgba(243, 156, 18, 0.8)'
                        ],
                        borderColor: [
                            'rgba(52, 152, 219, 1)',
                            'rgba(231, 76, 60, 1)',
                            'rgba(243, 156, 18, 1)'
                        ],
                        borderWidth: 2
                    }]
                },
                options: {
                    responsive: true,
                    plugins: {
                        title: {
                            display: true,
                            text: 'Your Browser vs Benchmark Results',
                            font: { size: 14, weight: 'bold' }
                        },
                        legend: { display: false }
                    },
                    scales: {
                        y: {
                            beginAtZero: true,
                            title: {
                                display: true,
                                text: 'Execution Time (ms)'
                            }
                        }
                    }
                }
            });
        }
    }
});
