$fa = 1;
$fs = 0.4;

module rcube(size, radius) {
    hull() {
        translate([radius, radius]) cylinder(r = radius, h = size[2]);
        translate([size[0] - radius, radius]) cylinder(r = radius, h = size[2]);
        translate([size[0] - radius, size[1] - radius]) cylinder(r = radius, h = size[2]);
        translate([radius, size[1] - radius]) cylinder(r = radius, h = size[2]);
    }
}

module epaper_holder() {
    difference() {
        translate([3,8-0.5-5,0]) rcube([136,105+5,30], 6);
        translate([13, 13, -1]) rcube([117, 87.5,35], 3);
        translate([8, 8.5, 1]) cube([126.5, 101,35]);

        translate([5.75,4+9,15]) cylinder(d=3,h=25);
        translate([143-6.5,4+9,15]) cylinder(d=3,h=25);
        translate([143-6.5,118-4-9,15]) cylinder(d=3,h=25);
        translate([5.75,118-4-9,15]) cylinder(d=3,h=25);
    
        translate([131,16.5,20]) cube([3.5+3.5, 6,6]);

        translate([131,6.5+34,10]) cube([3.5+3.5, 6,6]);
        translate([4,90,10]) cube([134, 6,6]);
        
        translate([4,20.5,20]) cube([26, 6,6]);
        translate([4,40.5,20]) cube([26, 6,6]);
        translate([4,60.5,20]) cube([26, 6,6]);
        
        translate([(143-129)/2-30, (118-103)/2, 1]) {
            translate([0,13+3,6]) rotate([0,90,0]) cylinder(h=117,d=6); 
            translate([0,33+3,6]) rotate([0,90,0]) cylinder(h=117,d=6); 
            translate([0,53+3,6]) rotate([0,90,0]) cylinder(h=117,d=6); 
        
        }
        translate([(143-129)/2-4, (118-103)/2, 1]) {
            translate([1,13-0.5,3-0.5]) cube([60,7,7]);
            translate([1,33-0.5,3-0.5]) cube([60,7,7]);
            translate([1,53-0.5,3-0.5]) cube([60,7,7]);
        }
    }
}

epaper_holder();
