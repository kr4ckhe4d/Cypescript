// EXPECT: 'super' can only be used inside a class method
function loose(): void { super.anything(); }
loose();
