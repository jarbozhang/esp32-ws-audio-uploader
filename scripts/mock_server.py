import asyncio
import websockets
import json
import threading
import sys

# Event to signal the main loop to broadcast a hook
broadcast_queue = asyncio.Queue()

async def input_loop():
    while True:
        line = await asyncio.to_thread(sys.stdin.readline)
        if not line:
            break
        cmd = line.strip()
        if cmd == 'q':
            sys.exit(0)
        elif cmd == 'p':
            await broadcast_queue.put("PermissionRequest")
        elif cmd == 'f':
            await broadcast_queue.put("PostToolUseFailure")
        elif cmd == 's':
            await broadcast_queue.put("Stop")
        else:
            print(f"Unknown command: {cmd}")
            print("Commands: p (Permission), f (Failure), s (Stop), q (Quit)")

async def handler(websocket):
    print(f"Client connected: {websocket.remote_address}")
    try:
        async for message in websocket:
            if isinstance(message, str):
                # JSON message
                try:
                    data = json.loads(message)
                    print(f"Received JSON: {data.get('type')}")
                    if data.get('type') == 'start':
                        print(f"  Start params: {data}")
                    elif data.get('type') == 'end':
                        print("  End received. Sending Ack & Result.")
                        await websocket.send(json.dumps({"type": "ack", "reqId": data.get("reqId")}))
                        await websocket.send(json.dumps({"type": "result", "reqId": data.get("reqId"), "text": "Mock transcript"}))
                except json.JSONDecodeError:
                    print(f"Received text (invalid JSON): {message}")
            elif isinstance(message, bytes):
                # Binary audio
                print(f"Received Audio: {len(message)} bytes")
    except websockets.ConnectionClosed:
        print("Client disconnected")

async def broadcaster(server):
    while True:
        event_name = await broadcast_queue.get()
        print(f"Broadcasting hook: {event_name}")
        event_json = json.dumps({
            "type": "hook",
            "id": f"mock-{event_name}",
            "hook_event_name": event_name,
            "ts": 1234567890
        })
        
        # websockets.serve returns a server object.
        # We need to track connected clients manually or use the server object if it exposes them?
        # The 'websockets' library server object has a 'websockets' property which is a set of connected sockets.
        if server.websockets:
            for ws in server.websockets:
                try:
                    await ws.send(event_json)
                except:
                    pass
        else:
            print("No clients connected to receive broadcast.")

async def main():
    print("Starting Mock ASR Server on 0.0.0.0:8765")
    print("Commands: p (Permission), f (Failure), s (Stop), q (Quit)")
    
    async with websockets.serve(handler, "0.0.0.0", 8765) as server:
        # Start input loop and broadcaster
        asyncio.create_task(input_loop())
        asyncio.create_task(broadcaster(server))
        await asyncio.Future()  # Run forever

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
