# ChatBot Setup Guide

## Project Structure

```
ChatBot/
├── Backend/                    # Spring Boot application
│   ├── src/
│   │   ├── main/java/com/chatbot/
│   │   │   ├── ChatbotApplication.java
│   │   │   ├── common/         # Request ID, filters
│   │   │   ├── config/         # Spring configuration
│   │   │   ├── controller/     # REST endpoints
│   │   │   ├── exception/      # Error handling
│   │   │   ├── model/          # Entities & DTOs
│   │   │   ├── repository/     # Data access
│   │   │   └── service/        # Business logic
│   │   └── resources/
│   │       └── application.yml # Configuration
│   ├── pom.xml
│   └── Dockerfile
├── Frontend/                   # React application
│   ├── src/
│   │   ├── components/         # React components
│   │   ├── hooks/              # Custom hooks
│   │   ├── styles/             # CSS files
│   │   ├── api.js              # API service
│   │   ├── App.jsx
│   │   └── main.jsx
│   ├── package.json
│   ├── vite.config.js
│   ├── index.html
│   └── Dockerfile
├── docker-compose.yml          # All services
├── ARCHITECTURE.md             # Design documentation
├── BACKEND_IMPLEMENTATION.md   # Backend details
├── FRONTEND_IMPLEMENTATION.md  # Frontend details
├── DEPLOYMENT_GUIDE.md         # Production setup
└── README.md                   # Overview

```

## Prerequisites

Before starting, ensure you have installed:

- Docker & Docker Compose (for containerized deployment)
- Node.js 18+ & npm (for frontend development)
- Java 17+ & Maven (for backend development)
- Git

## Quick Start with Docker (Recommended)

### Step 1: Start All Services

```bash
cd /path/to/ChatBot
docker-compose up -d
```

This will start:
- PostgreSQL database (port 5432)
- Ollama LLM server (port 11434)
- Spring Boot backend (port 8080)
- React frontend (port 3000)

### Step 2: Download LLM Model

```bash
docker exec chatbot-ollama ollama pull llama2
```

Available models:
- `llama2` (4.4GB) - Recommended for most users
- `mistral` (4.7GB) - High quality responses
- `neural-chat` (4.7GB) - Optimized for conversations

### Step 3: Access the Application

Open your browser and navigate to:
```
http://localhost:3000
```

### Step 4: Check Services Status

```bash
# Check all containers
docker-compose ps

# View backend logs
docker-compose logs -f backend

# View frontend logs
docker-compose logs -f frontend

# View Ollama logs
docker-compose logs -f ollama
```

## Local Development Setup

### Backend Development

```bash
cd Backend

# Install dependencies
mvn install

# Run with h2 database (in-memory)
mvn spring-boot:run -Dspring-boot.run.arguments="--spring.profiles.active=dev"

# Run tests
mvn test

# Build JAR
mvn clean package
```

Backend will start at `http://localhost:8080`

### Frontend Development

```bash
cd Frontend

# Install dependencies
npm install

# Start development server with hot reload
npm run dev

# Build for production
npm run build

# Preview production build
npm run preview

# Run linter
npm run lint
```

Frontend will start at `http://localhost:3000`

## Configuration

### Environment Variables

Copy `.env.example` to `.env` and update values:

```bash
cp .env.example .env
```

### Backend Configuration (application.yml)

```yaml
spring:
  datasource:
    url: jdbc:postgresql://localhost:5432/chatbot
    username: chatbot_user
    password: chatbot_password

ollama:
  api:
    url: http://localhost:11434
    timeout: 300
  models:
    - name: llama2
```

### Frontend Configuration (vite.config.js)

```javascript
server: {
  proxy: {
    '/api': {
      target: 'http://backend:8080',
      changeOrigin: true
    }
  }
}
```

## API Endpoints

### Chat Operations

```
POST /api/chat/send
  Send message to chatbot
  Body: {
    "conversationId": "uuid",
    "message": "Hello!",
    "model": "llama2"
  }

POST /api/chat/conversations
  Create new conversation
  Body: { "model": "llama2" }

GET /api/chat/conversations
  Get all conversations

GET /api/chat/history/{conversationId}
  Get conversation history

DELETE /api/chat/history/{conversationId}
  Delete conversation

GET /api/chat/health
  Health check endpoint
```

## Docker Commands

```bash
# Start all services
docker-compose up -d

# Stop all services
docker-compose down

# View logs
docker-compose logs -f [service-name]

# Rebuild containers
docker-compose build

# Remove volumes (careful!)
docker-compose down -v

# Execute command in container
docker exec chatbot-backend bash

# View service status
docker-compose ps

# Pull latest images
docker-compose pull
```

## Troubleshooting

### Backend won't start

```bash
# Check if port 8080 is available
lsof -i :8080

# Check logs
docker-compose logs backend

# Rebuild container
docker-compose build --no-cache backend
docker-compose up backend
```

### Frontend can't connect to backend

```bash
# Ensure backend is running
curl http://localhost:8080/api/chat/health

# Check frontend proxy configuration in vite.config.js
# Inside Docker, use: http://backend:8080
# Locally, use: http://localhost:8080
```

### Ollama not responding

```bash
# Check Ollama container
docker exec chatbot-ollama curl http://localhost:11434/api/tags

# Pull a model
docker exec chatbot-ollama ollama pull llama2

# Check available disk space
docker exec chatbot-ollama du -sh ~/.ollama
```

### Database issues

```bash
# Connect to PostgreSQL
docker exec -it chatbot-postgres psql -U chatbot_user -d chatbot

# Check database size
SELECT pg_size_pretty(pg_database_size('chatbot'));

# Reset database
docker-compose down -v
docker-compose up postgres
```

## Request ID Tracing

Every request includes a unique `X-Request-ID` header:

```
[req_a1b2c3d4] INFO - Request started
[req_a1b2c3d4] INFO - Processing message
[req_a1b2c3d4] INFO - Calling Ollama
[req_a1b2c3d4] INFO - Response received
```

This enables end-to-end tracing through:
- Frontend (console)
- Backend (logs)
- Database (audit trail)

## Production Deployment

See `DEPLOYMENT_GUIDE.md` for:
- Kubernetes deployment
- SSL/TLS configuration
- Performance tuning
- Monitoring and logging
- Scaling strategies

## Common Issues

### Out of Memory

If services crash due to OOM:

```bash
# Increase Docker memory limit
# In docker-compose.yml, add:
services:
  backend:
    mem_limit: 1g
```

### Slow Responses

1. Check Ollama model size
2. Increase Ollama timeout in configuration
3. Monitor CPU and memory usage
4. Scale horizontally with multiple instances

### CORS Errors

Ensure backend CORS is configured correctly:

```java
configuration.setAllowedOrigins(Arrays.asList(
    "http://localhost:3000",
    "http://localhost:5173"
));
```

## Performance Optimization

### Backend
- Implement caching for conversation history
- Use connection pooling for database
- Enable query logging for slow queries
- Monitor thread pool settings

### Frontend
- Use React.memo for message components
- Implement virtual scrolling for large lists
- Lazy load conversation history
- Optimize bundle size with code splitting

### Ollama
- Keep models in memory with persistent volumes
- Monitor GPU memory usage
- Adjust context window settings
- Consider quantized models for smaller devices

## Testing

### Backend Tests

```bash
cd Backend
mvn test

# Run specific test
mvn test -Dtest=ChatServiceTest

# With coverage
mvn test jacoco:report
```

### Frontend Tests

```bash
cd Frontend
npm test

# With coverage
npm test -- --coverage
```

## Next Steps

1. Read `ARCHITECTURE.md` for system design
2. Read `BACKEND_IMPLEMENTATION.md` for backend details
3. Read `FRONTEND_IMPLEMENTATION.md` for frontend details
4. Read `DEPLOYMENT_GUIDE.md` for production setup
5. Check logs: `docker-compose logs -f`

## Support

For issues or questions:
1. Check logs for error messages
2. Review configuration files
3. Verify all services are running
4. Consult troubleshooting section

---

**Status:** ✅ Production-Ready
**Last Updated:** June 23, 2026
**Version:** 1.0.0
