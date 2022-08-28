//
// Simple Mounting Plate for Pi Platter and 2 cell battery on NORSMIC
// 7" touch LCD screen
//

thickness = 1.5;

//
// Plate 1 with Pi mounting holes
//
module plate1() {
    difference() {
        // Plate
        cube([91, 80, thickness]);
        
        
    }
}

//
// Complete assembly
//
difference() {
    // Base Plate
    cube([126, 105, thickness]);
    
    
    // LCD Mounting holes (mount plate to LCD standoffs)
    translate([34, 29, -0.5]) {
        cylinder(h=thickness+1, r=1.15, $fn=120);
    }
    translate([92, 29, -0.5]) {
        cylinder(h=thickness+1, r=1.15, $fn=120);
    }
    translate([34, 78, -0.5]) {
        cylinder(h=thickness+1, r=1.15, $fn=120);
    }
    translate([92, 78, -0.5]) {
        cylinder(h=thickness+1, r=1.15, $fn=120);
    }
    
    // Pi Platter Mounting holes
    translate([59, 78, -0.5]) {
        cylinder(h=thickness+1, r=1.625, $fn=120);
    }
    translate([117, 78, -0.5]) {
        cylinder(h=thickness+1, r=1.625, $fn=120);
    }
    translate([59, 101, -0.5]) {
        cylinder(h=thickness+1, r=1.625, $fn=120);
    }
    translate([117, 101, -0.5]) {
        cylinder(h=thickness+1, r=1.625, $fn=120);
    }
    
    // Battery zip-tie cut-outs
    translate([-0.5, 10, -0.5]) {
        cube([2, 4, thickness+1]);
    }
    translate([-0.5, 54, -0.5]) {
        cube([2, 4, thickness+1]);
    }
    translate([43, 10, -0.5]) {
        cube([2, 4, thickness+1]);
    }
    translate([43, 54, -0.5]) {
        cube([2, 4, thickness+1]);
    }
    
    // Remove unnecessary material
    translate([50, -0.5, -0.5]) {
        cube([91.5, 25.5, thickness+1]);
    }
    translate([-0.5, 82, -0.5]) {
        cube([30, 25.5, thickness+1]);
    }
}