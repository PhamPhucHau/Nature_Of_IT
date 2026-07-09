#!/bin/bash

# ChatBot Docker Deployment Script

CHATBOT_DIR="/Users/hauphamphuc/Documents/Application/Knowleage and AI/Nature_Of_IT/AI/ChatBot"

echo "=========================================="
echo "ChatBot Docker Deployment Script"
echo "=========================================="
echo ""

# Change to ChatBot directory
cd "$CHATBOT_DIR" || exit 1

echo "[1/6] Stopping any running containers..."
docker-compose down -v 2>/dev/null || true
sleep 2

echo "[2/6] Building Docker images (this may take 5-10 minutes)..."
docker-compose build --no-cache

if [ $? -ne 0 ]; then
    echo "ERROR: Docker build failed!"
    exit 1
fi

echo "[3/6] Starting services..."
docker-compose up -d

echo "[4/6] Waiting for services to be ready (30 seconds)..."
sleep 30

echo "[5/6] Checking service health..."
echo "Backend: $(curl -s http://localhost:8080/api/chat/health | jq -r '.status // "OFFLINE"' 2>/dev/null || echo "OFFLINE")"
echo "Frontend: $(curl -s http://localhost:3000 -I 2>/dev/null | grep -i "200" && echo "ONLINE" || echo "OFFLINE")"
echo "Ollama: $(curl -s http://localhost:11434/api/tags 2>/dev/null | jq -r '.status // "OFFLINE"' 2>/dev/null || echo "OFFLINE")"

echo "[6/6] Pulling Ollama model (llama2)..."
docker exec chatbot-ollama ollama pull llama2

echo ""
echo "=========================================="
echo "✓ Deployment Complete!"
echo "=========================================="
echo ""
echo "Services:"
echo "  Frontend:  http://localhost:3000"
echo "  Backend:   http://localhost:8080"
echo "  Ollama:    http://localhost:11434"
echo ""
echo "Logs:"
echo "  docker-compose logs -f backend"
echo "  docker-compose logs -f frontend"
echo "  docker-compose logs -f ollama"
echo ""
