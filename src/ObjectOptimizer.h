#pragma once

#include <map>
#include <string>
#include <vector>
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"

// Phase 1 Optimization: Direct Property Access
class ObjectOptimizer {
public:
    struct PropertyInfo {
        size_t offset;
        llvm::Type* type;
        std::string typeName;
    };
    
    struct ObjectLayout {
        std::map<std::string, PropertyInfo> properties;
        size_t totalSize;
        llvm::StructType* structType;  // Changed from llvm::Type*
    };
    
private:
    std::map<std::string, ObjectLayout> objectLayouts;
    std::map<std::string, llvm::Value*> propertyCache;
    
public:
    // Create optimized object layout at compile time
    ObjectLayout createObjectLayout(
        const std::vector<std::pair<std::string, std::string>>& properties,
        llvm::LLVMContext& context
    );
    
    // Generate direct property access (no hash map lookups)
    llvm::Value* generateDirectPropertyAccess(
        llvm::IRBuilder<>& builder,
        llvm::Value* objectPtr,
        const std::string& property,
        const ObjectLayout& layout
    );
    
    // Inline constant properties at compile time
    llvm::Value* tryInlineProperty(
        const std::string& objectKey,
        const std::string& property
    );
    
    // Cache frequently accessed properties
    void cachePropertyAccess(
        const std::string& cacheKey,
        llvm::Value* value
    );
    
    llvm::Value* getCachedProperty(const std::string& cacheKey);
    
    // Clear cache when needed
    void clearCache();
};

// Optimized object creation with struct layout
class OptimizedObjectCreator {
public:
    // Create object as LLVM struct instead of hash map storage
    llvm::Value* createOptimizedObject(
        llvm::IRBuilder<>& builder,
        llvm::LLVMContext& context,
        llvm::Module* module,
        const ObjectOptimizer::ObjectLayout& layout,
        const std::vector<llvm::Value*>& propertyValues
    );
    
    // Generate struct type for object
    llvm::StructType* generateObjectStruct(
        llvm::LLVMContext& context,
        const ObjectOptimizer::ObjectLayout& layout
    );
};
