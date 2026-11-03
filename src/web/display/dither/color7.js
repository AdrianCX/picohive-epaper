var EPAPER_PALETTE = [
    [  0,   0,   0],
    [255, 255, 255],
    [  0, 255,   0],
    [  0,   0, 255],
    [255,   0,   0],
    [255, 255,   0],
    [255, 128,   0]
];

function packImage(imgData, width, height) {
    var out = new Uint8Array(width * height / 2);
    for (let i = 0; i < width * height / 2; i++) {
        var index = i * 8;
        out[i] = (getEpaperColor(imgData[index], imgData[index+1], imgData[index+2]) << 4)
               + getEpaperColor(imgData[index+4], imgData[index+5], imgData[index+6]);
    }
    return out;
}
