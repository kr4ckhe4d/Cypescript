// Complex Data Structures Example - E-commerce System
// Demonstrates advanced usage of arrays and objects in Cypescript

println("=== E-commerce Management System ===");

// Product catalog with nested data structures
let catalog = {
    storeName: "CyberMart",
    categories: [
        {
            name: "Electronics",
            products: [
                {
                    id: 1001,
                    name: "Laptop Pro",
                    price: 1299,
                    inStock: true,
                    specs: {
                        cpu: "Intel i7",
                        ram: 16,
                        storage: "512GB SSD"
                    },
                    reviews: [
                        { rating: 5, comment: "Excellent performance!" },
                        { rating: 4, comment: "Great value for money" }
                    ]
                },
                {
                    id: 1002,
                    name: "Smartphone X",
                    price: 899,
                    inStock: false,
                    specs: {
                        screen: "6.1 inch OLED",
                        camera: "48MP Triple",
                        battery: "4000mAh"
                    },
                    reviews: [
                        { rating: 5, comment: "Amazing camera quality" },
                        { rating: 5, comment: "Battery lasts all day" }
                    ]
                }
            ]
        },
        {
            name: "Books",
            products: [
                {
                    id: 2001,
                    name: "Programming Fundamentals",
                    price: 49,
                    inStock: true,
                    specs: {
                        pages: 450,
                        author: "Jane Smith",
                        publisher: "TechBooks"
                    },
                    reviews: [
                        { rating: 4, comment: "Clear explanations" },
                        { rating: 5, comment: "Perfect for beginners" }
                    ]
                }
            ]
        }
    ]
};

// Customer orders system
let orders = [
    {
        orderId: "ORD-001",
        customer: {
            name: "Alice Johnson",
            email: "alice@email.com",
            address: {
                street: "123 Main St",
                city: "Tech City",
                zipCode: "12345"
            }
        },
        items: [
            { productId: 1001, quantity: 1, unitPrice: 1299 },
            { productId: 2001, quantity: 2, unitPrice: 49 }
        ],
        status: "shipped",
        total: 1397
    },
    {
        orderId: "ORD-002",
        customer: {
            name: "Bob Wilson",
            email: "bob@email.com",
            address: {
                street: "456 Oak Ave",
                city: "Code Town",
                zipCode: "67890"
            }
        },
        items: [
            { productId: 1002, quantity: 1, unitPrice: 899 }
        ],
        status: "pending",
        total: 899
    }
];

// Display store information
print("Store: ");
println(catalog.storeName);
print("Categories Available: ");
println(catalog.categories.length);

println("");
println("=== Product Catalog ===");

// Electronics category analysis
let electronics = catalog.categories[0];
print("Category: ");
println(electronics.name);
print("Products in stock: ");

let inStockCount: i32 = 0;
if (electronics.products[0].inStock == true) {
    inStockCount = inStockCount + 1;
}
if (electronics.products[1].inStock == true) {
    inStockCount = inStockCount + 1;
}
println(inStockCount);

// Featured product details
let laptop = electronics.products[0];
println("Featured Product:");
print("  Name: ");
println(laptop.name);
print("  Price: $");
println(laptop.price);
print("  CPU: ");
println(laptop.specs.cpu);
print("  RAM: ");
print(laptop.specs.ram);
println("GB");
print("  Storage: ");
println(laptop.specs.storage);

// Customer reviews analysis
println("Customer Reviews:");
print("  Review 1: ");
print(laptop.reviews[0].rating);
print(" stars - ");
println(laptop.reviews[0].comment);
print("  Review 2: ");
print(laptop.reviews[1].rating);
print(" stars - ");
println(laptop.reviews[1].comment);

// Books category
let books = catalog.categories[1];
let programmingBook = books.products[0];
println("");
print("Book Category - Featured: ");
println(programmingBook.name);
print("  Author: ");
println(programmingBook.specs.author);
print("  Pages: ");
println(programmingBook.specs.pages);
print("  Price: $");
println(programmingBook.price);

println("");
println("=== Order Management ===");

// Process orders
let totalRevenue: i32 = 0;
let totalOrders: i32 = orders.length;

// Order 1 analysis
let order1 = orders[0];
print("Order ID: ");
println(order1.orderId);
print("Customer: ");
println(order1.customer.name);
print("Email: ");
println(order1.customer.email);
print("City: ");
println(order1.customer.address.city);
print("Items ordered: ");
println(order1.items.length);
print("Order total: $");
println(order1.total);
print("Status: ");
println(order1.status);

totalRevenue = totalRevenue + order1.total;

// Order 2 analysis
let order2 = orders[1];
print("Order ID: ");
println(order2.orderId);
print("Customer: ");
println(order2.customer.name);
print("Status: ");
println(order2.status);
print("Order total: $");
println(order2.total);

totalRevenue = totalRevenue + order2.total;

println("");
println("=== Business Analytics ===");
print("Total Orders: ");
println(totalOrders);
print("Total Revenue: $");
println(totalRevenue);

// Inventory status
println("Inventory Status:");
if (laptop.inStock == true) {
    print("✓ ");
    print(laptop.name);
    println(" - Available");
} else {
    print("✗ ");
    print(laptop.name);
    println(" - Out of Stock");
}

let smartphone = electronics.products[1];
if (smartphone.inStock == true) {
    print("✓ ");
    print(smartphone.name);
    println(" - Available");
} else {
    print("✗ ");
    print(smartphone.name);
    println(" - Out of Stock");
}

if (programmingBook.inStock == true) {
    print("✓ ");
    print(programmingBook.name);
    println(" - Available");
}

println("");
println("=== System Analysis Complete ===");
println("E-commerce platform successfully managing:");
println("- Multi-level product catalogs");
println("- Customer order processing");
println("- Inventory tracking");
println("- Revenue analytics");
println("- Review management");

println("");
println("Cypescript handles complex nested data structures efficiently!");
