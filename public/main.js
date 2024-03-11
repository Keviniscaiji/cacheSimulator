const socket = new WebSocket('ws://localhost:8080');
        
socket.addEventListener('open', function (event) {
    console.log('Connected to WS Server');
});

socket.addEventListener('message', function (event) {
    console.log('Message from server ', event.data);
    document.getElementById('output').innerHTML = `<p>${event.data}</p>`;
});
function isValidHex(input) {
    const regex = /^0x[0-9a-fA-F]{16}$/;
    return regex.test(input);
}

function submitForm1() {
    
    var blockSize = document.getElementById("blockSize").value;
    var setSize = document.getElementById("setSize").value;
    var cacheSizeL1 = document.getElementById("cacheSizeL1").value;
    var cacheSizeL2 = document.getElementById("cacheSizeL2").value;
    var disablePrefetcher = document.getElementById("disablePrefetcher").checked;
    var useL2Cache = document.getElementById("useL2Cache").checked;

    var inputData = `${blockSize},${setSize},${cacheSizeL1},${cacheSizeL2},${disablePrefetcher},${useL2Cache}`;
    // alert(inputData);
    fetch('/process', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ input: inputData }),
    })
        .then(response => response.json())
        .then(data => {
            document.getElementById('output').innerHTML = `<p>${data.message}</p>`;

            document.getElementById('blockSize').disabled = true;
            document.getElementById('setSize').disabled = true;
            document.getElementById('cacheSizeL1').disabled = true;
            document.getElementById('cacheSizeL2').disabled = true;
            document.getElementById('disablePrefetcher').disabled = true;
            document.getElementById('useL2Cache').disabled = true;
            document.getElementById('parameterButton').disabled = true;


            document.getElementById('operation').disabled = false;
            document.getElementById('address').disabled = false;
            document.getElementById('sendSimButton').disabled = false;
            document.getElementById('sendStopButton').disabled = false;
        })
        .catch(error => console.error('Error:', error));

}


function submitForm2() {
    var operation = document.getElementById("operation").value;
    var address = document.getElementById("address").value;
    if (operation != "R" && operation != "W") {
        alert("Invalid operation. Please enter R for read or W for write.");
        return;
    }
    if (!isValidHex(address)) {
        alert("Invalid address. Please enter a valid 64-bit hexadecimal address.");
        return;
    }
    var inputData = `${operation} ${address}`;

    fetch('/process', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ input: inputData }),
    })
        .then(response => response.json())
        .then(data => {
            // document.getElementById('output').innerHTML += `<p>${data.output}</p>`;
        })
        .catch(error => console.error('Error:', error));
}

function stopSimulation() {
    fetch('/process', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ input: "stop" }),
    })
        .then(response => response.json())
        .then(data => {

            document.getElementById('blockSize').disabled = true;
            document.getElementById('setSize').disabled = true;
            document.getElementById('cacheSizeL1').disabled = true;
            document.getElementById('cacheSizeL2').disabled = true;
            document.getElementById('disablePrefetcher').disabled = true;
            document.getElementById('useL2Cache').disabled = true;
            document.getElementById('parameterButton').disabled = true;
            document.getElementById('operation').disabled = true;
            document.getElementById('address').disabled = true;
            document.getElementById('sendSimButton').disabled = true;
            document.getElementById('sendStopButton').disabled = true;
        })
        .catch(error => console.error('Error:', error));
}
