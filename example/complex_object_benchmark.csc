// Extreme Complex Object Benchmark
// Testing native TypeScript-style objects with complex data structures

println("🚀 Starting Extreme Complex Object Benchmark");
println("============================================");

// Create complex employee management system
let employee1 = {
    firstName: "Alice",
    lastName: "Johnson",
    employeeId: 12345,
    department: "Engineering",
    position: "Senior Software Engineer",
    salary: 125000,
    yearsExperience: 8,
    isActive: true,
    isRemote: true,
    hasSecurityClearance: false,
    performanceRating: 95,
    projectCount: 12,
    teamSize: 6,
    certificationCount: 4,
    bonusEligible: true
};

let employee2 = {
    firstName: "Bob",
    lastName: "Smith", 
    employeeId: 12346,
    department: "Product",
    position: "Product Manager",
    salary: 115000,
    yearsExperience: 6,
    isActive: true,
    isRemote: false,
    hasSecurityClearance: true,
    performanceRating: 88,
    projectCount: 8,
    teamSize: 4,
    certificationCount: 2,
    bonusEligible: true
};

let employee3 = {
    firstName: "Carol",
    lastName: "Davis",
    employeeId: 12347,
    department: "Design",
    position: "UX Designer",
    salary: 95000,
    yearsExperience: 5,
    isActive: true,
    isRemote: true,
    hasSecurityClearance: false,
    performanceRating: 92,
    projectCount: 15,
    teamSize: 3,
    certificationCount: 3,
    bonusEligible: false
};

let employee4 = {
    firstName: "David",
    lastName: "Wilson",
    employeeId: 12348,
    department: "Engineering",
    position: "DevOps Engineer",
    salary: 110000,
    yearsExperience: 7,
    isActive: false,
    isRemote: true,
    hasSecurityClearance: true,
    performanceRating: 90,
    projectCount: 10,
    teamSize: 5,
    certificationCount: 6,
    bonusEligible: true
};

let employee5 = {
    firstName: "Eva",
    lastName: "Brown",
    employeeId: 12349,
    department: "Marketing",
    position: "Marketing Director",
    salary: 135000,
    yearsExperience: 10,
    isActive: true,
    isRemote: false,
    hasSecurityClearance: false,
    performanceRating: 97,
    projectCount: 20,
    teamSize: 8,
    certificationCount: 5,
    bonusEligible: true
};

println("✅ Created 5 complex employee objects");

// Intensive property access benchmark
println("🔥 Starting intensive property access benchmark...");

let iterations: i32 = 10000;
let counter: i32 = 0;

// Benchmark loop - accessing multiple properties per iteration
while (counter < iterations) {
    // Employee 1 property access
    let name1: string = employee1.firstName;
    let dept1: string = employee1.department;
    let salary1: i32 = employee1.salary;
    let active1: i32 = employee1.isActive;
    let rating1: i32 = employee1.performanceRating;
    
    // Employee 2 property access
    let name2: string = employee2.firstName;
    let dept2: string = employee2.department;
    let salary2: i32 = employee2.salary;
    let active2: i32 = employee2.isActive;
    let rating2: i32 = employee2.performanceRating;
    
    // Employee 3 property access
    let name3: string = employee3.firstName;
    let dept3: string = employee3.department;
    let salary3: i32 = employee3.salary;
    let active3: i32 = employee3.isActive;
    let rating3: i32 = employee3.performanceRating;
    
    // Employee 4 property access
    let name4: string = employee4.firstName;
    let dept4: string = employee4.department;
    let salary4: i32 = employee4.salary;
    let active4: i32 = employee4.isActive;
    let rating4: i32 = employee4.performanceRating;
    
    // Employee 5 property access
    let name5: string = employee5.firstName;
    let dept5: string = employee5.department;
    let salary5: i32 = employee5.salary;
    let active5: i32 = employee5.isActive;
    let rating5: i32 = employee5.performanceRating;
    
    counter = counter + 1;
}

println("✅ Completed 10,000 iterations");
println("📊 Total property accesses: 250,000");
println("🎯 Complex object benchmark complete!");

// Summary statistics
println("");
println("📈 Benchmark Summary:");
println("- Objects created: 5 complex objects");
println("- Properties per object: 15 properties");
println("- Iterations: 10,000");
println("- Property accesses per iteration: 25");
println("- Total property accesses: 250,000");
println("- Mixed data types: strings, integers, booleans");
println("🚀 Native TypeScript-style objects performing at scale!");
