// Complex Cypescript Example
// Demonstrates arithmetic, comparisons, and nested control flow

let score: i32 = 85;
let bonus: i32 = 15;
let penalty: i32 = 5;

// Calculate final score with complex arithmetic
let finalScore: i32 = score + bonus - penalty;
let maxScore: i32 = 100;
let passingGrade: i32 = 60;

print("=== Grade Calculator ===");
print("Base score: ");
print(score);
print("Bonus points: ");
print(bonus);
print("Penalty points: ");
print(penalty);
print("Final score: ");
print(finalScore);

// Complex nested grading logic
if (finalScore >= 90) {
    print("Grade: A - Excellent!");
    
    if (finalScore == maxScore) {
        print("Perfect score achieved!");
    } else {
        let pointsFromPerfect: i32 = maxScore - finalScore;
        print("Points from perfect: ");
        print(pointsFromPerfect);
    }
} else {
    if (finalScore >= 80) {
        print("Grade: B - Good work!");
        
        let pointsToA: i32 = 90 - finalScore;
        print("Points needed for A: ");
        print(pointsToA);
    } else {
        if (finalScore >= 70) {
            print("Grade: C - Satisfactory");
        } else {
            if (finalScore >= passingGrade) {
                print("Grade: D - Passing");
                print("Consider studying more!");
            } else {
                print("Grade: F - Failed");
                print("Retake required");
                
                let pointsToPass: i32 = passingGrade - finalScore;
                print("Points needed to pass: ");
                print(pointsToPass);
            }
        }
    }
}

// Additional calculations
let doubled: i32 = finalScore * 2;
let halved: i32 = finalScore / 2;
let remainder: i32 = finalScore % 10;

print("=== Score Analysis ===");
print("Doubled score: ");
print(doubled);
print("Half score: ");
print(halved);
print("Last digit: ");
print(remainder);

// Comparison demonstrations
if (finalScore > score) {
    print("Final score improved from base!");
} else {
    print("Final score decreased from base");
}

if (finalScore != score) {
    print("Score was modified by bonuses/penalties");
}

if (finalScore <= maxScore) {
    print("Score is within valid range");
}

print("=== Program Complete ===");
