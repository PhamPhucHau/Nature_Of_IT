# Docker Deployment Fix & Alternative Setup

## Issue Encountered

**Error:** Docker images not found
```
ERROR: openjdk:17-jdk-slim: not found
ERROR: maven:3.8.1-openjdk-17: not found
```

## Root Cause

The Docker image tags specified in the Dockerfile are:
- `openjdk:17-jdk-slim` - Not available in default registry
- `maven:3.8.1-openjdk-17` - Not available in default registry

## Solution 1: Use Alternative Images (Recommended)

### Update Backend/Dockerfile

Replace the content with:

```dockerfile
# Build stage
FROM maven:3.9-eclipse-temurin-17 AS builder

WORKDIR /build

# Copy pom.xml and download dependencies
COPY pom.xml .
RUN mvn dependency:go-offline

# Copy source code
COPY src ./src

# Build the application
RUN mvn clean package -DskipTests

# Runtime stage
FROM eclipse-temurin:17-jdk-alpine

WORKDIR /app

# Copy the JAR from builder
COPY --from=builder /build/target/chatbot-backend-1.0.0.jar app.jar

# Expose port
EXPOSE 8080

# Set environment
ENV JAVA_OPTS="-Xmx512m -Xms256m"

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=40s --retries=3 \
    CMD wget --quiet --tries=1 --spider http://localhost:8080/api/chat/health || exit 1

# Run the application
ENTRYPOINT ["sh", "-c", "java $JAVA_OPTS -jar app.jar"]
```

### Update Frontend/Dockerfile

Replace the content with:

```dockerfile
# Build stage
FROM node:20-alpine AS builder

WORKDIR /app

# Copy package files
COPY package*.json ./

# Install dependencies
RUN npm install

# Copy source
COPY . .

# Build app
RUN npm run build

# Runtime stage
FROM node:20-alpine

WORKDIR /app

# Install serve to run the static app
RUN npm install -g serve

# Copy built app from builder
COPY --from=builder /app/dist ./dist

EXPOSE 3000

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=10s --retries=3 \
    CMD wget --quiet --tries=1 --spider http://localhost:3000 || exit 1

# Serve the app
CMD ["serve", "-s", "dist", "-l", "3000"]
```

---

## Solution 2: Use Pre-built Images (Faster)

### Update docker-compose.yml

```yaml
version: '3.8'

services:
  # PostgreSQL Database
  postgres:
    image: postgres:15-alpine
    container_name: chatbot-postgres
    environment:
      POSTGRES_DB: chatbot
      POSTGRES_USER: chatbot_user
      POSTGRES_PASSWORD: chatbot_password
    ports:
      - "5432:5432"
    volumes:
      - postgres_data:/var/lib/postgresql/data
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U chatbot_user"]
      interval: 10s
      timeout: 5s
      retries: 5
    networks:
      - chatbot-network

  # Ollama LLM Server
  ollama:
    image: ollama/ollama:latest
    container_name: chatbot-ollama
    ports:
      - "11434:11434"
    volumes:
      - ollama_data:/root/.ollama
    environment:
      OLLAMA_HOST: 0.0.0.0:11434
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:11434/api/tags"]
      interval: 10s
      timeout: 5s
      retries: 5
    networks:
      - chatbot-network

  # Backend service (using pre-built or local image)
  backend:
    build:
      context: ./Backend
      dockerfile: Dockerfile
    container_name: chatbot-backend
    depends_on:
      postgres:
        condition: service_healthy
      ollama:
        condition: service_healthy
    environment:
      SPRING_DATASOURCE_URL: jdbc:postgresql://postgres:5432/chatbot
      SPRING_DATASOURCE_USERNAME: chatbot_user
      SPRING_DATASOURCE_PASSWORD: chatbot_password
      SPRING_JPA_HIBERNATE_DDL_AUTO: update
      OLLAMA_API_URL: http://ollama:11434
      SPRING_PROFILES_ACTIVE: prod
    ports:
      - "8080:8080"
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8080/api/chat/health"]
      interval: 10s
      timeout: 5s
      retries: 5
    networks:
      - chatbot-network

  # Frontend service
  frontend:
    build:
      context: ./Frontend
      dockerfile: Dockerfile
    container_name: chatbot-frontend
    depends_on:
      - backend
    ports:
      - "3000:3000"
    healthcheck:
      test: ["CMD", "wget", "--quiet", "--tries=1", "--spider", "http://localhost:3000"]
      interval: 10s
      timeout: 5s
      retries: 5
    networks:
      - chatbot-network

volumes:
  postgres_data:
    driver: local
  ollama_data:
    driver: local

networks:
  chatbot-network:
    driver: bridge
```

---

## Solution 3: Manual Development Setup (For Testing)

If Docker build is causing issues, you can run services individually:

### 1. Start PostgreSQL

```bash
docker run -d \
  --name chatbot-postgres \
  -e POSTGRES_DB=chatbot \
  -e POSTGRES_USER=chatbot_user \
  -e POSTGRES_PASSWORD=chatbot_password \
  -p 5432:5432 \
  postgres:15-alpine
```

### 2. Start Ollama

```bash
docker run -d \
  --name chatbot-ollama \
  -p 11434:11434 \
  ollama/ollama:latest

# Pull model
docker exec chatbot-ollama ollama pull llama2
```

### 3. Start Backend (Local Java)

```bash
cd Backend

# Build
mvn clean install

# Run
mvn spring-boot:run -Dspring-boot.run.arguments="--spring.profiles.active=dev"
```

Backend will start at: `http://localhost:8080`

### 4. Start Frontend (Local Node)

```bash
cd Frontend

# Install dependencies
npm install

# Run dev server
npm run dev
```

Frontend will start at: `http://localhost:3000`

---

## Step-by-Step Deployment Guide

### Prerequisites

```bash
# 1. Check Docker
docker --version  # v28.0+
docker-compose --version  # v2.0+

# 2. Check available disk space
df -h | grep /Users

# 3. Check Docker daemon is running
docker ps
```

### Deployment Steps

**Step 1: Update Dockerfiles**

Replace `Backend/Dockerfile` and `Frontend/Dockerfile` with the versions above.

**Step 2: Build Images**

```bash
cd /path/to/ChatBot

# Option A: Build both
docker-compose build

# Option B: Build specific
docker-compose build backend
docker-compose build frontend
```

**Step 3: Start Services**

```bash
# Start in detached mode
docker-compose up -d

# Monitor startup
watch docker-compose ps

# Check logs
docker-compose logs -f
```

**Step 4: Pull LLM Model**

```bash
docker exec chatbot-ollama ollama pull llama2
```

Wait 5-10 minutes for model download (~4GB).

**Step 5: Verify Services**

```bash
# Check all running
docker-compose ps

# Health checks
curl http://localhost:8080/api/chat/health
curl http://localhost:3000
curl http://localhost:11434/api/tags
```

**Step 6: Access Application**

- Frontend: http://localhost:3000
- Backend: http://localhost:8080/api
- Ollama: http://localhost:11434

---

## Troubleshooting

### Build Failure: Image Not Found

**Error:**
```
ERROR: image:tag: not found
```

**Solution:**
1. Update image names to available versions
2. Pull images manually: `docker pull eclipse-temurin:17-jdk-alpine`
3. Check Docker Hub for available tags

### Container Fails to Start

**Error:**
```
container exited with code 1
```

**Solution:**
```bash
# Check logs
docker-compose logs service-name

# Restart service
docker-compose restart service-name

# Rebuild
docker-compose build --no-cache service-name
```

### Out of Memory

**Error:**
```
killed with signal 9
```

**Solution:**
Add memory limits to docker-compose.yml:
```yaml
services:
  backend:
    mem_limit: 1g
    memswap_limit: 2g
```

### Port Already in Use

**Error:**
```
bind: address already in use
```

**Solution:**
```bash
# Find process on port
lsof -i :8080

# Kill process
kill -9 PID

# Or use different port in docker-compose.yml
ports:
  - "8081:8080"  # Change host port
```

---

## Performance Optimization

### Image Size Reduction

```dockerfile
# Use minimal base images
FROM eclipse-temurin:17-jdk-alpine  # ~200MB
FROM node:20-alpine                  # ~100MB

# Remove unnecessary packages
RUN apk add --no-cache curl

# Multi-stage builds reduce final size
```

### Build Caching

```bash
# Build with cache
docker-compose build --build-arg BUILD_DATE=$(date -u +'%Y-%m-%dT%H:%M:%SZ')

# Invalidate cache if needed
docker-compose build --no-cache
```

### Startup Optimization

```yaml
# Parallel startup
services:
  postgres:
    healthcheck:
      start_period: 10s  # Faster startup detection
```

---

## Verification Checklist

- [ ] Docker version 20.10+
- [ ] Docker Compose version 1.29+
- [ ] 8GB+ RAM available
- [ ] 20GB+ disk space
- [ ] Dockerfiles updated with correct images
- [ ] docker-compose build succeeds
- [ ] docker-compose up -d starts all services
- [ ] docker-compose ps shows all healthy
- [ ] Health endpoints respond (200)
- [ ] Frontend loads at localhost:3000
- [ ] Backend responds at localhost:8080
- [ ] Ollama has model pulled
- [ ] Can create conversation
- [ ] Can send/receive message
- [ ] Request IDs in logs

---

## Quick Commands Reference

```bash
# Start everything
docker-compose up -d

# View status
docker-compose ps

# View logs
docker-compose logs -f
docker-compose logs -f backend
docker-compose logs -f frontend

# Stop everything
docker-compose down

# Full reset (remove data)
docker-compose down -v

# Rebuild
docker-compose build --no-cache

# Restart service
docker-compose restart backend

# Execute command
docker exec chatbot-backend bash

# Pull model
docker exec chatbot-ollama ollama pull llama2

# List models
docker exec chatbot-ollama ollama list
```

---

**Updated:** July 4, 2026
**Status:** ✅ Ready for Deployment

