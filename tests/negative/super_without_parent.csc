// EXPECT: 'super' used in class 'Lone', which does not extend anything
class Lone {
    speak(): void { super.speak(); }
}
let l: Lone = new Lone();
l.speak();
