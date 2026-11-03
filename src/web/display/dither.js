function floydSteinbergDithering(context, palArr, w, h, errorMultiplier = 0.9)
{
    var imgData = context.getImageData(0, 0, w, h);
    var data = imgData.data;

    for(var y = 0; y < h; y++)
    {
        for (var x = 0; x < w; x++)
        {
            var id = ((y * w) + x) * 4;
            var oldpixel = [data[id], data[id + 1], data[id + 2]];

            var newpixel = findClosest(oldpixel, palArr);

            data[id] = newpixel[0];
            data[id + 1] = newpixel[1];
            data[id + 2] = newpixel[2];

            data[ id + 3 ] = 255;

            var quantErr = getQuantErr(oldpixel, newpixel);
            var err;

            id = ((y * w) + (x + 1)) * 4;
            if (id < data.length) {
                err = applyErr([data[id], data[id + 1], data[id + 2]], quantErr, (7/16), errorMultiplier);
                data[id] = err[0];
                data[id + 1] = err[1];
                data[id + 2] = err[2];
            }

            id = (((y + 1) * w) + (x - 1)) * 4;
            if (id < data.length) {
                err = applyErr([data[id], data[id + 1], data[id + 2]], quantErr, (3/16), errorMultiplier);
                data[id] = err[0];
                data[id + 1] = err[1];
                data[id + 2] = err[2];
            }

            id = (((y + 1) * w) + x) * 4;
            if (id < data.length) {
                err = applyErr([data[id], data[id + 1], data[id + 2]], quantErr, (5/16), errorMultiplier);
                data[id] = err[0];
                data[id + 1] = err[1];
                data[id + 2] = err[2];
            }

            id = (((y + 1) * w) + (x + 1)) * 4;
            if (id < data.length) {
                err = applyErr([data[id], data[id + 1], data[id + 2]], quantErr, (1/16), errorMultiplier);
                data[id] = err[0];
                data[id + 1] = err[1];
                data[id + 2] = err[2];
            }
        }
    }
    context.putImageData(imgData, 0, 0)
}

function findClosest(oldpixel, palArr) {
    var maxDist = 766;
    var idx = 0;
    for (var i = 0; i < palArr.length; i++) {
        var dist = Math.abs(oldpixel[0] - palArr[i][0]) + Math.abs(oldpixel[1] - palArr[i][1]) + Math.abs(oldpixel[2] - palArr[i][2]);
        if (dist < maxDist) {
            maxDist = dist;
            idx = i;
        }
    }
    return palArr[idx];
}

function getQuantErr(oldpixel, newpixel) {
    oldpixel[0] -= newpixel[0];
    oldpixel[1] -= newpixel[1];
    oldpixel[2] -= newpixel[2];
    return oldpixel;
}

function applyErr(pixel, error, factor, multiplier) {
    pixel[0] += error[0] * factor * multiplier;
    pixel[1] += error[1] * factor * multiplier;
    pixel[2] += error[2] * factor * multiplier;
    return(pixel);
}
