#include "ObjectOptimizer.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"

// Create optimized object layout at compile time
ObjectOptimizer::ObjectLayout ObjectOptimizer::createObjectLayout(
    const std::vector<std::pair<std::string, std::string>>& properties,
    llvm::LLVMContext& context) {
    
    ObjectLayout layout;
    size_t currentOffset = 0;
    
    std::vector<llvm::Type*> structMembers;
    
    for (size_t i = 0; i < properties.size(); i++) {
        const auto& prop = properties[i];
        PropertyInfo info;
        info.offset = currentOffset;
        info.typeName = prop.second;
        
        // Determine LLVM type and size
        if (prop.second == "string") {
            info.type = llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0);
            currentOffset += sizeof(void*); // Pointer size
        } else if (prop.second == "i32" || prop.second == "boolean") {
            info.type = llvm::Type::getInt32Ty(context);
            currentOffset += 4; // 32-bit integer
        } else if (prop.second == "f64") {
            info.type = llvm::Type::getDoubleTy(context);
            currentOffset += 8; // 64-bit double
        } else {
            // Default to i32
            info.type = llvm::Type::getInt32Ty(context);
            currentOffset += 4;
        }
        
        // Store in order-preserving vector
        layout.properties.push_back({prop.first, info});
        // Store index for fast lookup
        layout.propertyIndices[prop.first] = i;
        
        structMembers.push_back(info.type);
    }
    
    layout.totalSize = currentOffset;
    
    // Create LLVM struct type for this object layout
    layout.structType = llvm::StructType::create(
        context,
        structMembers,
        "CypescriptObject",
        false // not packed
    );
    
    return layout;
}

// Generate direct property access (no hash map lookups)
llvm::Value* ObjectOptimizer::generateDirectPropertyAccess(
    llvm::IRBuilder<>& builder,
    llvm::Value* objectPtr,
    const std::string& property,
    const ObjectLayout& layout) {
    
    // Find property index using fast lookup
    auto indexIt = layout.propertyIndices.find(property);
    if (indexIt == layout.propertyIndices.end()) {
        return nullptr; // Property not found
    }
    
    size_t memberIndex = indexIt->second;
    const PropertyInfo& propInfo = layout.properties[memberIndex].second;
    
    // Generate unique instruction names to avoid LLVM IR conflicts
    static size_t instructionCounter = 0;
    instructionCounter++;
    
    std::string ptrName = property + "_ptr_" + std::to_string(instructionCounter);
    std::string valName = property + "_val_" + std::to_string(instructionCounter);
    
    // Generate direct struct member access - CRITICAL OPTIMIZATION
    // This replaces 4+ hash map lookups with a single GEP instruction!
    std::vector<llvm::Value*> indices = {
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(builder.getContext()), 0), // struct base
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(builder.getContext()), memberIndex) // member index
    };
    
    llvm::Value* memberPtr = builder.CreateGEP(
        layout.structType,
        objectPtr,
        indices,
        ptrName
    );
    
    // Load the value - single instruction instead of multiple lookups
    llvm::Value* value = builder.CreateLoad(propInfo.type, memberPtr, valName);
    
    return value;
}

// Inline constant properties at compile time
llvm::Value* ObjectOptimizer::tryInlineProperty(
    const std::string& objectKey,
    const std::string& property) {
    
    // Phase 2 optimization: For compile-time constant objects, 
    // we can inline property values completely eliminating runtime overhead
    
    // TODO: Implement constant folding for known object values
    // This will provide massive speedup for constant objects
    return nullptr;
}

// Cache frequently accessed properties
void ObjectOptimizer::cachePropertyAccess(
    const std::string& cacheKey,
    llvm::Value* value) {
    
    propertyCache[cacheKey] = value;
}

llvm::Value* ObjectOptimizer::getCachedProperty(const std::string& cacheKey) {
    auto it = propertyCache.find(cacheKey);
    if (it != propertyCache.end()) {
        return it->second;
    }
    return nullptr;
}

void ObjectOptimizer::clearCache() {
    propertyCache.clear();
}

// Optimized object creation with struct layout
llvm::Value* OptimizedObjectCreator::createOptimizedObject(
    llvm::IRBuilder<>& builder,
    llvm::LLVMContext& context,
    llvm::Module* module,
    const ObjectOptimizer::ObjectLayout& layout,
    const std::vector<llvm::Value*>& propertyValues) {
    
    // Allocate struct on stack (much faster than heap allocation)
    llvm::Value* objectPtr = builder.CreateAlloca(
        layout.structType,
        nullptr,
        "optimized_object"
    );
    
    // Initialize struct members directly - no hash map overhead
    for (size_t i = 0; i < layout.properties.size() && i < propertyValues.size(); i++) {
        std::vector<llvm::Value*> indices = {
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0),
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), i)
        };
        
        llvm::Value* memberPtr = builder.CreateGEP(
            layout.structType,
            objectPtr,
            indices,
            layout.properties[i].first + "_init_ptr"
        );
        
        builder.CreateStore(propertyValues[i], memberPtr);
    }
    
    return objectPtr;
}

llvm::StructType* OptimizedObjectCreator::generateObjectStruct(
    llvm::LLVMContext& context,
    const ObjectOptimizer::ObjectLayout& layout) {
    
    return layout.structType;
}
