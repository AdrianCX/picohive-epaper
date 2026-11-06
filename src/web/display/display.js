var dataCopy = new Uint8Array(0);

const EPAPER_WIDTH = {{width}};
const EPAPER_HEIGHT = {{height}};

function downloadImage() {
    const blob = new Blob( [dataCopy], {type: "application/octet-stream"});
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.style.display = 'none';
    a.href = url;
    a.download = 'epaper.bin';
    document.body.appendChild(a);
    a.click();

    window.URL.revokeObjectURL(url);
    document.body.removeChild(a);
}

function getEpaperColor(r,g,b) {
    const r1 = Math.floor(r/100);
    const g1 = Math.floor(g/100);
    const b1 = Math.floor(b/100);

    if ((r1 == 0) && (g1 == 0) && (b1 == 0)) {
        return 0;
    }

    if ((r1 == 1) && (g1 == 1) && (b1 == 1)) {
        return 3;
    }

    if ((g1 == 0) && (b1 == 0)) {
        return 4;
    }

    if ((r1 == 0) && (b1 == 0)) {
        return 2;
    }

    if ((r1 == 0) && (g1 == 0)) {
        return 3;
    }

    if ((r1 == g1) && (b1 == 0)) {
        return 5;
    }

    if ((r1 == 2) && (g1 == 1) && (b1 == 0)) {
        return 6;
    }

    return 1;
}

var PREVIEW_RGB = [
    [  0,   0,   0],
    [255, 255, 255],
    [  0, 125,   0],
    [128, 128, 128],
    [225,   0,   0],
    [225, 225,   0],
    [255, 128,   0],
];

function loadImage() {
    var input, file, fr, img;

    if (typeof window.FileReader !== 'function') {
        console.log("File API unavailable");
        return;
    }

    input = document.getElementById('imgfile');
    if (!input) {
        console.log("imgfile missing");
        return;
    }
    else if (!input.files) {
        console.log("missing input.files");
        return;
    }
    else if (!input.files[0]) {
        console.log("no file selected");
        return;
    }
    else {
        file = input.files[0];
        fr = new FileReader();
        fr.onload = createImage;
        fr.readAsDataURL(file);
    }

    function createImage() {
        img = new Image();
        img.onload = imageLoaded;
        img.src = fr.result;
    }

    function imageLoaded() {
        var canvas = document.getElementById("canvas")

        var curWidth = img.width;
        var curHeight = img.height;

        var leftOffset = 0;
        var topOffset = 0;

        const WIDTH = EPAPER_WIDTH;
        const HEIGHT = EPAPER_HEIGHT;

        canvas.width = WIDTH;
        canvas.height = HEIGHT;

        var ctx = canvas.getContext("2d");

        if (document.getElementById("stretch").checked)
        {
            curWidth = WIDTH;
            curHeight = HEIGHT;
        }
        else if (document.getElementById("calendarize").checked)
        {
            curWidth = 270;
            curHeight = HEIGHT;

            ctx.fillStyle = "white";
            ctx.fillRect(0, 0, canvas.width, canvas.height);
        }
        else
        {
            if ((curWidth/curHeight) > (WIDTH/HEIGHT))
            {
                curHeight = curHeight * WIDTH/curWidth;
                curWidth = WIDTH;
                topOffset = (HEIGHT - curHeight) / 2;
            }
            else
            {
                curWidth = curWidth * HEIGHT/curHeight;
                curHeight = HEIGHT;
                leftOffset = (WIDTH - curWidth) / 2;
            }
        }

        ctx.drawImage(img,leftOffset, topOffset, curWidth, curHeight);

        var e = document.getElementById("algorithm");
        if (e.value == 1) {
            floydSteinbergDithering(ctx, EPAPER_PALETTE, WIDTH, HEIGHT);
        }

        var imgSource = ctx.getImageData(0, 0, WIDTH, HEIGHT);
        var imgData = imgSource.data;

        dataCopy = packImage(imgData, WIDTH, HEIGHT);

        for (let i = 0; i < WIDTH*HEIGHT; i++)
        {
            var index = i*4;
            var rgb = PREVIEW_RGB[getEpaperColor(imgData[index], imgData[index+1], imgData[index+2])];
            imgData[index] = rgb[0];
            imgData[index+1] = rgb[1];
            imgData[index+2] = rgb[2];
        }

        ctx.putImageData(imgSource, 0, 0);
    }
}

function showImage() {
    var blob = new Blob(["/rc/image/show\0", dataCopy]);
    handler.send(blob, T("lbl.project"));
}

function uploadImage() {
    var blob = new Blob(["/rc/image/save/", document.getElementById("imageslot").value, '\0', dataCopy]);
    handler.send(blob, T("lbl.inscribe"));
}

function sendSlotCommand(command, label) {
    handler.send(new Blob(["/rc/", command, "/", document.getElementById("imageslot").value]), T(label));
}

function loadUploadedImage() {
    sendSlotCommand("image/load", "lbl.resurrect");
}

function calendarDemo() {
    sendSlotCommand("calendar/demo", "lbl.demo");
}

function calendarShow() {
    sendSlotCommand("calendar/show", "lbl.calshow");
}

function showCommandReply(path) {
    onMessage(path, (body) => hudToast(body.startsWith("error") ? "bad" : "ok", path + ": " + body));
}

["/rc/image/show", "/rc/image/save", "/rc/image/load", "/rc/calendar/demo", "/rc/calendar/show"].forEach(showCommandReply);
