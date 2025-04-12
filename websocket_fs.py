from fastapi import FastAPI
from fastapi import WebSocket , WebSocketDisconnect
from fastapi.responses import FileResponse
import uvicorn

app = FastAPI()

clients : WebSocket = []

@app.get("/download")
def Krooz():
     return FileResponse(
        path="Krooz.zip",
        filename=f"Krooz.zip",
        media_type="application/zip",
    )

@app.websocket("/ws")
async def Sock(websocket: WebSocket):
    await websocket.accept()
    await websocket.send_text("You Are Connected to The Server")
    clients.append(websocket)
    try:
        while True:
            data =  await websocket.receive_text()
            for client in clients:
                    await client.send_text(data)

    except WebSocketDisconnect:
        clients.remove(websocket)
    except Exception:
        clients.remove(websocket)

if __name__ == "__main__":
     uvicorn.run(app=app)
