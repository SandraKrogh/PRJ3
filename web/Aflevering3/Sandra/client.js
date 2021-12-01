//Note: Route is chat 
const socket = new WebSocket('ws://localhost:8000/chat');

//Connection opened 
socket.addEventListener('open', function (event){
    //Sending a message to the web socket server
    socket.send('Hello Server!');
});

//Listen for Messages
socket.addEventListener('message', function(message)
{
    console.log('Message from server', message,data);
});