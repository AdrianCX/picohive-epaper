var EPAPER_PALETTE = [
    [  0,   0,   0],
    [255, 255, 255],
    [255,   0,   0],
];

function packImage(imgData, width, height) {
    var out = new Uint8Array(width * height / 4);
    var offset = Math.floor(width * height / 8);
    for (let i = 0; i < width * height; i++) {
        var index = i * 4;
        var outIndex = Math.floor(i / 8);
        var color = getEpaperColor(imgData[index], imgData[index+1], imgData[index+2]);
        out[outIndex] = (out[outIndex] << 1) + ((color == 0) ? 0 : 1);
        out[outIndex + offset] = (out[outIndex + offset] << 1) + ((color == 4) ? 1 : 0);
    }
    return out;
}
