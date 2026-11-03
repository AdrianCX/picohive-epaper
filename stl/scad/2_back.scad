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

module epaper_back() {

    difference() {
        translate([0,0,0]) {
            translate([3,8-0.5-5,0]) rcube([136,105+5,2], 6);
            translate([8, 8.5, -3]) cube([126.5, 101,3]);
        }

        translate([10, 10.5, -4]) cube([122.5, 97,3]);

        translate([5.75,4+9,0]) cylinder(d=3,h=1.5);
        translate([143-6.5,4+9,0]) cylinder(d=3,h=1.5);
        translate([143-6.5,118-4-9,0]) cylinder(d=3,h=1.5);
        translate([5.75,118-4-9,0]) cylinder(d=3,h=1.5);
    
    }

}

translate([0, 0, 34]) epaper_back();
