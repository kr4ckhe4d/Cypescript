// Complex Data Processing - Native Compiler Compatible
// Demonstrates advanced programming patterns using current language features

println("=== Advanced Data Processing System ===");

// Company data simulation using individual variables
let companyName: string = "TechCorp Industries";
let companyFounded: i32 = 2020;
let totalDepartments: i32 = 2;

println("Company Information:");
print("Name: ");
println(companyName);
print("Founded: ");
println(companyFounded);
print("Departments: ");
println(totalDepartments);

println("");
println("=== Engineering Department Analysis ===");

// Engineering department data
let engDeptName: string = "Engineering";
let engBudget: i32 = 500000;
let engEmployeeCount: i32 = 2;

// Employee 1 - Alice Johnson
let alice_name: string = "Alice Johnson";
let alice_role: string = "Senior Developer";
let alice_level: i32 = 45;
let alice_salary: i32 = 95000;

// Alice's skills (simulated as individual variables)
let alice_skill1: string = "JavaScript";
let alice_skill2: string = "Python";
let alice_skill3: string = "React";

// Alice's projects
let alice_project1: string = "WebApp";
let alice_project2: string = "API Gateway";

// Employee 2 - Bob Smith
let bob_name: string = "Bob Smith";
let bob_role: string = "DevOps Engineer";
let bob_level: i32 = 42;
let bob_salary: i32 = 85000;

// Bob's skills
let bob_skill1: string = "Docker";
let bob_skill2: string = "Kubernetes";
let bob_skill3: string = "AWS";

// Bob's projects
let bob_project1: string = "Infrastructure";
let bob_project2: string = "CI/CD";

print("Department: ");
println(engDeptName);
print("Budget: $");
println(engBudget);
print("Team Size: ");
println(engEmployeeCount);

println("");
println("Team Members:");

// Display Alice's information
print("1. ");
println(alice_name);
print("   Role: ");
println(alice_role);
print("   Level: ");
println(alice_level);
print("   Salary: $");
println(alice_salary);
print("   Primary Skill: ");
println(alice_skill1);
print("   Secondary Skills: ");
print(alice_skill2);
print(", ");
println(alice_skill3);
print("   Main Project: ");
println(alice_project1);
print("   Secondary Project: ");
println(alice_project2);

println("");

// Display Bob's information
print("2. ");
println(bob_name);
print("   Role: ");
println(bob_role);
print("   Level: ");
println(bob_level);
print("   Salary: $");
println(bob_salary);
print("   Specialization: ");
println(bob_skill2);
print("   Cloud Platform: ");
println(bob_skill3);
print("   Infrastructure Project: ");
println(bob_project1);
print("   Automation Project: ");
println(bob_project2);

println("");
println("=== Marketing Department Analysis ===");

// Marketing department data
let mktDeptName: string = "Marketing";
let mktBudget: i32 = 200000;
let mktEmployeeCount: i32 = 1;

// Employee - Carol Davis
let carol_name: string = "Carol Davis";
let carol_role: string = "Marketing Manager";
let carol_level: i32 = 38;
let carol_salary: i32 = 75000;

// Carol's skills
let carol_skill1: string = "SEO";
let carol_skill2: string = "Content Marketing";
let carol_skill3: string = "Analytics";

// Carol's projects
let carol_project1: string = "Brand Campaign";
let carol_project2: string = "Social Media";

print("Department: ");
println(mktDeptName);
print("Budget: $");
println(mktBudget);
print("Team Size: ");
println(mktEmployeeCount);

println("");
println("Team Member:");
print("1. ");
println(carol_name);
print("   Role: ");
println(carol_role);
print("   Level: ");
println(carol_level);
print("   Salary: $");
println(carol_salary);
print("   Expertise: ");
println(carol_skill1);
print("   Content Skills: ");
println(carol_skill2);
print("   Data Analysis: ");
println(carol_skill3);
print("   Campaign Project: ");
println(carol_project1);
print("   Digital Project: ");
println(carol_project2);

println("");
println("=== Company-Wide Analysis ===");

// Calculate totals
let totalBudget: i32 = engBudget + mktBudget;
let totalEmployees: i32 = engEmployeeCount + mktEmployeeCount;
let totalSalaries: i32 = alice_salary + bob_salary + carol_salary;
let averageSalary: i32 = totalSalaries / totalEmployees;

print("Total Budget: $");
println(totalBudget);
print("Total Employees: ");
println(totalEmployees);
print("Total Salaries: $");
println(totalSalaries);
print("Average Salary: $");
println(averageSalary);

println("");
println("=== Skill Matrix Analysis ===");

// Skill analysis using conditionals
println("Technical Skills Distribution:");

// JavaScript developers
print("JavaScript: ");
if (alice_skill1 == "JavaScript") {
    println(alice_name);
} else {
    println("No specialists found");
}

// Python developers
print("Python: ");
if (alice_skill2 == "Python") {
    println(alice_name);
} else {
    println("No specialists found");
}

// Cloud infrastructure
print("Cloud Infrastructure: ");
if (bob_skill3 == "AWS") {
    print(bob_name);
    println(" (AWS Certified)");
} else {
    println("No specialists found");
}

// Container technology
print("Container Technology: ");
if (bob_skill1 == "Docker") {
    print(bob_name);
    println(" (Docker Expert)");
} else {
    println("No specialists found");
}

// Marketing skills
print("SEO Expertise: ");
if (carol_skill1 == "SEO") {
    println(carol_name);
} else {
    println("No specialists found");
}

println("");
println("=== Project Portfolio ===");

// Project counting and analysis
let totalProjects: i32 = 6; // 2 per person
print("Total Active Projects: ");
println(totalProjects);

println("Project Breakdown:");
println("Engineering Projects:");
print("  - ");
println(alice_project1);
print("  - ");
println(alice_project2);
print("  - ");
println(bob_project1);
print("  - ");
println(bob_project2);

println("Marketing Projects:");
print("  - ");
println(carol_project1);
print("  - ");
println(carol_project2);

println("");
println("=== Performance Metrics ===");

// Performance analysis using loops and conditionals
let highPerformers: i32 = 0;
let seniorLevel: i32 = 40;

// Check Alice's performance
if (alice_level >= seniorLevel) {
    highPerformers = highPerformers + 1;
    print("High Performer: ");
    print(alice_name);
    print(" (Level ");
    print(alice_level);
    println(")");
}

// Check Bob's performance
if (bob_level >= seniorLevel) {
    highPerformers = highPerformers + 1;
    print("High Performer: ");
    print(bob_name);
    print(" (Level ");
    print(bob_level);
    println(")");
}

// Check Carol's performance
if (carol_level >= seniorLevel) {
    highPerformers = highPerformers + 1;
    print("High Performer: ");
    print(carol_name);
    print(" (Level ");
    print(carol_level);
    println(")");
}

print("Total High Performers: ");
println(highPerformers);

println("");
println("=== Salary Analysis ===");

// Salary brackets analysis
let highSalaryThreshold: i32 = 90000;
let midSalaryThreshold: i32 = 80000;

let highSalaryCount: i32 = 0;
let midSalaryCount: i32 = 0;
let regularSalaryCount: i32 = 0;

// Analyze Alice's salary
if (alice_salary >= highSalaryThreshold) {
    highSalaryCount = highSalaryCount + 1;
    print("High Salary Bracket: ");
    println(alice_name);
} else {
    if (alice_salary >= midSalaryThreshold) {
        midSalaryCount = midSalaryCount + 1;
    } else {
        regularSalaryCount = regularSalaryCount + 1;
    }
}

// Analyze Bob's salary
if (bob_salary >= highSalaryThreshold) {
    highSalaryCount = highSalaryCount + 1;
    print("High Salary Bracket: ");
    println(bob_name);
} else {
    if (bob_salary >= midSalaryThreshold) {
        midSalaryCount = midSalaryCount + 1;
        print("Mid Salary Bracket: ");
        println(bob_name);
    } else {
        regularSalaryCount = regularSalaryCount + 1;
    }
}

// Analyze Carol's salary
if (carol_salary >= highSalaryThreshold) {
    highSalaryCount = highSalaryCount + 1;
} else {
    if (carol_salary >= midSalaryThreshold) {
        midSalaryCount = midSalaryCount + 1;
    } else {
        regularSalaryCount = regularSalaryCount + 1;
        print("Regular Salary Bracket: ");
        println(carol_name);
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
println("=== Department Comparison ===");

// Compare departments
if (engBudget > mktBudget) {
    println("Engineering has the larger budget");
    let budgetDifference: i32 = engBudget - mktBudget;
    print("Budget difference: $");
    println(budgetDifference);
} else {
    println("Marketing has the larger budget");
}

if (engEmployeeCount > mktEmployeeCount) {
    println("Engineering has more employees");
} else {
    if (engEmployeeCount == mktEmployeeCount) {
        println("Departments have equal staff");
    } else {
        println("Marketing has more employees");
    }
}

println("");
println("=== System Summary ===");
println("✓ Company data processing complete");
println("✓ Employee analysis finished");
println("✓ Skill matrix generated");
println("✓ Project portfolio reviewed");
println("✓ Performance metrics calculated");
println("✓ Salary analysis completed");
println("✓ Department comparison done");

println("");
print("Data processing system handled ");
print(totalEmployees);
print(" employees across ");
print(totalDepartments);
println(" departments successfully!");

println("");
println("=== Advanced Processing Complete ===");
