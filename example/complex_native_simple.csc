// Complex Numerical Analysis - Native Compiler Compatible
// Demonstrates advanced programming patterns using integers and control flow

println("=== Advanced Numerical Analysis System ===");

// Company metrics using numerical data
let companyId: i32 = 1001;
let foundedYear: i32 = 2020;
let currentYear: i32 = 2024;
let companyAge: i32 = currentYear - foundedYear;

println("Company Analysis:");
print("Company ID: ");
println(companyId);
print("Founded: ");
println(foundedYear);
print("Age: ");
print(companyAge);
println(" years");

println("");
println("=== Department Budget Analysis ===");

// Department budgets
let engBudget: i32 = 500000;
let mktBudget: i32 = 200000;
let hrBudget: i32 = 150000;
let totalBudget: i32 = engBudget + mktBudget + hrBudget;

print("Engineering Budget: $");
println(engBudget);
print("Marketing Budget: $");
println(mktBudget);
print("HR Budget: $");
println(hrBudget);
print("Total Budget: $");
println(totalBudget);

// Budget percentage analysis
let engPercentage: i32 = (engBudget * 100) / totalBudget;
let mktPercentage: i32 = (mktBudget * 100) / totalBudget;
let hrPercentage: i32 = (hrBudget * 100) / totalBudget;

println("Budget Distribution:");
print("Engineering: ");
print(engPercentage);
println("%");
print("Marketing: ");
print(mktPercentage);
println("%");
print("HR: ");
print(hrPercentage);
println("%");

println("");
println("=== Employee Performance Metrics ===");

// Employee performance scores (0-100)
let alice_performance: i32 = 95;
let bob_performance: i32 = 88;
let carol_performance: i32 = 92;
let david_performance: i32 = 85;
let eve_performance: i32 = 90;

let totalEmployees: i32 = 5;
let totalPerformance: i32 = alice_performance + bob_performance + carol_performance + david_performance + eve_performance;
let averagePerformance: i32 = totalPerformance / totalEmployees;

println("Individual Performance Scores:");
print("Employee 1001 (Alice): ");
println(alice_performance);
print("Employee 1002 (Bob): ");
println(bob_performance);
print("Employee 1003 (Carol): ");
println(carol_performance);
print("Employee 1004 (David): ");
println(david_performance);
print("Employee 1005 (Eve): ");
println(eve_performance);

print("Average Performance: ");
println(averagePerformance);

println("");
println("=== Performance Classification ===");

// Performance tiers
let excellentThreshold: i32 = 90;
let goodThreshold: i32 = 80;
let satisfactoryThreshold: i32 = 70;

let excellentCount: i32 = 0;
let goodCount: i32 = 0;
let satisfactoryCount: i32 = 0;
let needsImprovementCount: i32 = 0;

// Classify Alice
if (alice_performance >= excellentThreshold) {
    excellentCount = excellentCount + 1;
    println("Alice: Excellent Performance");
} else {
    if (alice_performance >= goodThreshold) {
        goodCount = goodCount + 1;
    } else {
        if (alice_performance >= satisfactoryThreshold) {
            satisfactoryCount = satisfactoryCount + 1;
        } else {
            needsImprovementCount = needsImprovementCount + 1;
        }
    }
}

// Classify Bob
if (bob_performance >= excellentThreshold) {
    excellentCount = excellentCount + 1;
    println("Bob: Excellent Performance");
} else {
    if (bob_performance >= goodThreshold) {
        goodCount = goodCount + 1;
        println("Bob: Good Performance");
    } else {
        if (bob_performance >= satisfactoryThreshold) {
            satisfactoryCount = satisfactoryCount + 1;
        } else {
            needsImprovementCount = needsImprovementCount + 1;
        }
    }
}

// Classify Carol
if (carol_performance >= excellentThreshold) {
    excellentCount = excellentCount + 1;
    println("Carol: Excellent Performance");
} else {
    if (carol_performance >= goodThreshold) {
        goodCount = goodCount + 1;
    } else {
        if (carol_performance >= satisfactoryThreshold) {
            satisfactoryCount = satisfactoryCount + 1;
        } else {
            needsImprovementCount = needsImprovementCount + 1;
        }
    }
}

// Classify David
if (david_performance >= excellentThreshold) {
    excellentCount = excellentCount + 1;
} else {
    if (david_performance >= goodThreshold) {
        goodCount = goodCount + 1;
        println("David: Good Performance");
    } else {
        if (david_performance >= satisfactoryThreshold) {
            satisfactoryCount = satisfactoryCount + 1;
        } else {
            needsImprovementCount = needsImprovementCount + 1;
        }
    }
}

// Classify Eve
if (eve_performance >= excellentThreshold) {
    excellentCount = excellentCount + 1;
    println("Eve: Excellent Performance");
} else {
    if (eve_performance >= goodThreshold) {
        goodCount = goodCount + 1;
    } else {
        if (eve_performance >= satisfactoryThreshold) {
            satisfactoryCount = satisfactoryCount + 1;
        } else {
            needsImprovementCount = needsImprovementCount + 1;
        }
    }
}

println("Performance Distribution:");
print("Excellent (90+): ");
println(excellentCount);
print("Good (80-89): ");
println(goodCount);
print("Satisfactory (70-79): ");
println(satisfactoryCount);
print("Needs Improvement (<70): ");
println(needsImprovementCount);

println("");
println("=== Salary Analysis ===");

// Salary data
let alice_salary: i32 = 95000;
let bob_salary: i32 = 85000;
let carol_salary: i32 = 75000;
let david_salary: i32 = 80000;
let eve_salary: i32 = 88000;

let totalSalaries: i32 = alice_salary + bob_salary + carol_salary + david_salary + eve_salary;
let averageSalary: i32 = totalSalaries / totalEmployees;

println("Salary Information:");
print("Total Payroll: $");
println(totalSalaries);
print("Average Salary: $");
println(averageSalary);

// Salary brackets
let highSalaryThreshold: i32 = 90000;
let midSalaryThreshold: i32 = 80000;

let highSalaryCount: i32 = 0;
let midSalaryCount: i32 = 0;
let regularSalaryCount: i32 = 0;

// Analyze salaries
if (alice_salary >= highSalaryThreshold) {
    highSalaryCount = highSalaryCount + 1;
}
if (bob_salary >= highSalaryThreshold) {
    highSalaryCount = highSalaryCount + 1;
} else {
    if (bob_salary >= midSalaryThreshold) {
        midSalaryCount = midSalaryCount + 1;
    } else {
        regularSalaryCount = regularSalaryCount + 1;
    }
}
if (carol_salary >= highSalaryThreshold) {
    highSalaryCount = highSalaryCount + 1;
} else {
    if (carol_salary >= midSalaryThreshold) {
        midSalaryCount = midSalaryCount + 1;
    } else {
        regularSalaryCount = regularSalaryCount + 1;
    }
}
if (david_salary >= highSalaryThreshold) {
    highSalaryCount = highSalaryCount + 1;
} else {
    if (david_salary >= midSalaryThreshold) {
        midSalaryCount = midSalaryCount + 1;
    } else {
        regularSalaryCount = regularSalaryCount + 1;
    }
}
if (eve_salary >= highSalaryThreshold) {
    highSalaryCount = highSalaryCount + 1;
} else {
    if (eve_salary >= midSalaryThreshold) {
        midSalaryCount = midSalaryCount + 1;
    } else {
        regularSalaryCount = regularSalaryCount + 1;
    }
}

println("Salary Distribution:");
print("High Bracket ($90k+): ");
println(highSalaryCount);
print("Mid Bracket ($80k-$90k): ");
println(midSalaryCount);
print("Regular Bracket (<$80k): ");
println(regularSalaryCount);

println("");
println("=== Quarterly Growth Analysis ===");

// Quarterly revenue data
let q1_revenue: i32 = 250000;
let q2_revenue: i32 = 280000;
let q3_revenue: i32 = 320000;
let q4_revenue: i32 = 350000;

let annual_revenue: i32 = q1_revenue + q2_revenue + q3_revenue + q4_revenue;
let average_quarterly: i32 = annual_revenue / 4;

println("Quarterly Performance:");
print("Q1 Revenue: $");
println(q1_revenue);
print("Q2 Revenue: $");
println(q2_revenue);
print("Q3 Revenue: $");
println(q3_revenue);
print("Q4 Revenue: $");
println(q4_revenue);

print("Annual Revenue: $");
println(annual_revenue);
print("Average Quarterly: $");
println(average_quarterly);

// Growth analysis
let q1_to_q2_growth: i32 = q2_revenue - q1_revenue;
let q2_to_q3_growth: i32 = q3_revenue - q2_revenue;
let q3_to_q4_growth: i32 = q4_revenue - q3_revenue;

println("Growth Analysis:");
print("Q1 to Q2 Growth: $");
println(q1_to_q2_growth);
print("Q2 to Q3 Growth: $");
println(q2_to_q3_growth);
print("Q3 to Q4 Growth: $");
println(q3_to_q4_growth);

// Find best performing quarter
let bestQuarter: i32 = 1;
let bestRevenue: i32 = q1_revenue;

if (q2_revenue > bestRevenue) {
    bestQuarter = 2;
    bestRevenue = q2_revenue;
}
if (q3_revenue > bestRevenue) {
    bestQuarter = 3;
    bestRevenue = q3_revenue;
}
if (q4_revenue > bestRevenue) {
    bestQuarter = 4;
    bestRevenue = q4_revenue;
}

print("Best Performing Quarter: Q");
println(bestQuarter);
print("Best Quarter Revenue: $");
println(bestRevenue);

println("");
println("=== Complex Loop Analysis ===");

// Fibonacci sequence calculation
let fib_limit: i32 = 10;
let fib_a: i32 = 0;
let fib_b: i32 = 1;
let fib_count: i32 = 0;

println("Fibonacci Sequence Analysis:");
while (fib_count < fib_limit) {
    if (fib_count == 0) {
        print("F(0) = ");
        println(fib_a);
    } else {
        if (fib_count == 1) {
            print("F(1) = ");
            println(fib_b);
        } else {
            let fib_next: i32 = fib_a + fib_b;
            print("F(");
            print(fib_count);
            print(") = ");
            println(fib_next);
            fib_a = fib_b;
            fib_b = fib_next;
        }
    }
    fib_count = fib_count + 1;
}

println("");
println("=== Prime Number Analysis ===");

// Find prime numbers up to 30
let prime_limit: i32 = 30;
let prime_count: i32 = 0;

println("Prime Numbers Analysis:");
for (let num: i32 = 2; num <= prime_limit; num = num + 1) {
    let is_prime: i32 = 1; // 1 = true, 0 = false
    
    for (let divisor: i32 = 2; divisor * divisor <= num; divisor = divisor + 1) {
        let remainder: i32 = num % divisor;
        if (remainder == 0) {
            is_prime = 0;
        }
    }
    
    if (is_prime == 1) {
        print("Prime found: ");
        println(num);
        prime_count = prime_count + 1;
    }
}

print("Total primes found: ");
println(prime_count);

println("");
println("=== System Performance Summary ===");
println("✓ Budget analysis completed");
println("✓ Employee performance evaluated");
println("✓ Salary distribution calculated");
println("✓ Quarterly growth analyzed");
println("✓ Mathematical sequences computed");
println("✓ Prime number analysis finished");

print("Processed ");
print(totalEmployees);
print(" employees with ");
print(totalBudget);
println(" total budget");

print("System handled ");
print(fib_limit);
print(" Fibonacci numbers and ");
print(prime_count);
println(" prime calculations");

println("");
println("=== Advanced Analysis Complete ===");
println("Complex numerical processing successful!");
