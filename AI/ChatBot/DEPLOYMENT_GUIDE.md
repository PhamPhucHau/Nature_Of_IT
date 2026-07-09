# ChatBot Deployment Guide
## Complete Setup & Deployment Instructions

---

## ✅ Prerequisites

- Docker & Docker Compose installed
- Java 17+ installed
- Node.js 18+ installed
- npm or yarn package manager
- Git

---

## 📦 Docker Compose Setup

Create `docker-compose.yml` in project root:

```yaml
version: '3.8'

services:
  # Ollama LLM Server
  ollama:
    image: ollama/ollama:latest
    container_name: ollama-server
    ports:
      - "11434:11434"
    volumes:
      - ollama-data:/root/.ollama
    environment:
      - OLLAMA_HOST=0.0.0.0:11434
    command: serve
    networks:
      - chatbot-network
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:11434/api/tags"]
      interval: 10s
      timeout: 5s
      retries: 5

  # Java Spring Boot Backend
  backend:
    build:
      context: ./backend
      dockerfile: Dockerfile
    container_name: chatbot-backend
    ports:
      - "8080:8080"
    environment:
      - SPRING_PROFILES_ACTIVE=prod
      - OLLAMA_API_URL=http://ollama:11434
      - SPRING_DATASOURCE_URL=jdbc:postgresql://postgres:5432/chatbot
      - SPRING_DATASOURCE_USERNAME=chatbot_user
      - SPRING_DATASOURCE_PASSWORD=chatbot_password
    depends_on:
      ollama:
        condition: service_healthy
      postgres:
        condition: service_healthy
    networks:
      - chatbot-network
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8080/actuator/health"]
      interval: 10s
      timeout: 5s
      retries: 5

  # PostgreSQL Database
  postgres:
    image: postgres:15-alpine
    container_name: chatbot-postgres
    environment:
      - POSTGRES_DB=chatbot
      - POSTGRES_USER=chatbot_user
      - POSTGRES_PASSWORD=chatbot_password
    ports:
      - "5432:5432"
    volumes:
      - postgres-data:/var/lib/postgresql/data
    networks:
      - chatbot-network
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U chatbot_user"]
      interval: 10s
      timeout: 5s
      retries: 5

volumes:
  ollama-data:
  postgres-data:

networks:
  chatbot-network:
    driver: bridge
```

---

## 🐳 Backend Dockerfile

Create `backend/Dockerfile`:

```dockerfile
# Build stage
FROM maven:3.8.4-openjdk-17 as builder

WORKDIR /app

# Copy pom.xml and dependencies
COPY pom.xml .
RUN mvn dependency:go-offline

# Copy source code
COPY src ./src

# Build application
RUN mvn clean package -DskipTests

# Runtime stage
FROM openjdk:17-jdk-slim

WORKDIR /app

# Copy JAR from builder
COPY --from=builder /app/target/chatbot-backend-1.0.0.jar app.jar

# Expose port
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=40s --retries=3 \
  CMD java -cp app.jar org.springframework.boot.loader.PropertiesLauncher -c "curl -f http://localhost:8080/actuator/health || exit 1"

# Run application
ENTRYPOINT ["java", "-jar", "app.jar"]
```

---

## 🚀 Quick Start

### 1. Clone & Setup

```bash
# Clone repository
git clone <your-repo>
cd chatbot

# Create directory structure if not exists
mkdir -p frontend backend logs

# Copy configuration files
cp docker-compose.yml .
```

### 2. Start Services with Docker Compose

```bash
# Start all services
docker-compose up -d

# Check status
docker-compose ps

# View logs
docker-compose logs -f

# Specific service logs
docker-compose logs -f backend
docker-compose logs -f ollama
```

### 3. Pull Ollama Models

```bash
# Execute into ollama container
docker exec ollama-server ollama pull llama2

# Or pull other models
docker exec ollama-server ollama pull mistral
docker exec ollama-server ollama pull neural-chat
docker exec ollama-server ollama pull dolphin-mixtral

# List available models
docker exec ollama-server ollama list
```

### 4. Verify Services

```bash
# Check Ollama
curl http://localhost:11434/api/tags

# Check Backend
curl http://localhost:8080/actuator/health

# Check Database
psql -h localhost -U chatbot_user -d chatbot -c "SELECT version();"
```

---

## 🎨 Frontend Setup (Development)

```bash
# Navigate to frontend directory
cd frontend

# Install dependencies
npm install

# Start development server
npm run dev

# Frontend will be at http://localhost:3000 (or http://localhost:5173)
```

---

## 📝 Local Development Setup (No Docker)

### Backend

```bash
cd backend

# Build
mvn clean install

# Run (requires Ollama running separately)
mvn spring-boot:run

# Or run JAR
java -jar target/chatbot-backend-1.0.0.jar
```

### Frontend

```bash
cd frontend

# Install
npm install

# Run
npm run dev
```

### Ollama (Separate Terminal)

```bash
# Download and install Ollama from ollama.ai
# Then run:
ollama serve

# In another terminal, pull models:
ollama pull llama2
```

---

## 🔍 Testing API Endpoints

### Send Message

```bash
curl -X POST http://localhost:8080/api/chat/send \
  -H "Content-Type: application/json" \
  -H "X-Request-ID: test-req-123" \
  -d '{
    "message": "What is machine learning?",
    "model": "llama2",
    "conversationId": "conv-test-001"
  }'
```

### Get Chat History

```bash
curl -X GET http://localhost:8080/api/chat/history/conv-test-001 \
  -H "X-Request-ID: test-req-124"
```

### Clear Conversation

```bash
curl -X DELETE http://localhost:8080/api/chat/history/conv-test-001 \
  -H "X-Request-ID: test-req-125"
```

---

## 🛠️ Troubleshooting

### Issue: Ollama Connection Refused

```bash
# Check if Ollama is running
docker ps | grep ollama

# If not, start it
docker-compose up -d ollama

# Check Ollama logs
docker-compose logs ollama
```

### Issue: Backend Can't Connect to Database

```bash
# Check PostgreSQL connection
docker exec chatbot-postgres psql -U chatbot_user -d chatbot -c "SELECT 1"

# Check logs
docker-compose logs postgres
```

### Issue: Models Not Downloading

```bash
# SSH into Ollama container
docker exec -it ollama-server sh

# Try pulling model with verbose output
ollama pull llama2 -v

# List available models
ollama list
```

### Issue: Port Already in Use

```bash
# Find process using port
lsof -i :3000  # Frontend
lsof -i :8080  # Backend
lsof -i :11434 # Ollama
lsof -i :5432  # PostgreSQL

# Kill process
kill -9 <PID>

# Or use different port in docker-compose.yml
```

---

## 📊 Monitoring & Logs

### Backend Logs Location

```
# Inside container
/var/log/chatbot/chatbot.log

# View live logs
docker-compose logs -f backend --tail=100
```

### Check Health

```bash
# All services health
curl http://localhost:8080/actuator/health

# Detailed health
curl http://localhost:8080/actuator/health/details
```

---

## 🚢 Production Deployment

### Environment Setup

Create `.env` file:

```env
SPRING_PROFILES_ACTIVE=prod
OLLAMA_API_URL=http://ollama:11434
SPRING_DATASOURCE_URL=jdbc:postgresql://postgres:5432/chatbot
SPRING_DATASOURCE_USERNAME=chatbot_user
SPRING_DATASOURCE_PASSWORD=<strong-password>
JWT_SECRET=<strong-secret>
JAVA_OPTS=-Xmx1024m -Xms512m
```

### Production Docker Compose

```yaml
version: '3.8'

services:
  ollama:
    image: ollama/ollama:latest
    restart: always
    volumes:
      - /data/ollama:/root/.ollama
    ports:
      - "11434:11434"
    networks:
      - chatbot-network

  backend:
    image: chatbot-backend:1.0.0
    restart: always
    ports:
      - "8080:8080"
    env_file: .env
    depends_on:
      - ollama
      - postgres
    networks:
      - chatbot-network

  postgres:
    image: postgres:15-alpine
    restart: always
    env_file: .env
    volumes:
      - /data/postgres:/var/lib/postgresql/data
    networks:
      - chatbot-network

  nginx:
    image: nginx:alpine
    restart: always
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf:ro
      - /etc/letsencrypt:/etc/letsencrypt:ro
    depends_on:
      - backend
    networks:
      - chatbot-network

networks:
  chatbot-network:
    driver: bridge
```

### Nginx Configuration

Create `nginx.conf`:

```nginx
upstream backend {
    server backend:8080;
}

server {
    listen 80;
    server_name yourdomain.com;
    
    location / {
        proxy_pass http://backend;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Request-ID $request_id;
    }
    
    location /api/ {
        proxy_pass http://backend;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Request-ID $request_id;
    }
}
```

---

## 📈 Scaling

### Horizontal Scaling (Multiple Backend Instances)

```yaml
# docker-compose.yml with multiple backends
services:
  backend-1:
    build:
      context: ./backend
      dockerfile: Dockerfile
    ports:
      - "8081:8080"
    
  backend-2:
    build:
      context: ./backend
      dockerfile: Dockerfile
    ports:
      - "8082:8080"
    
  backend-3:
    build:
      context: ./backend
      dockerfile: Dockerfile
    ports:
      - "8083:8080"
    
  # Nginx load balancer
  nginx:
    image: nginx:alpine
    ports:
      - "8080:80"
    volumes:
      - ./nginx-lb.conf:/etc/nginx/nginx.conf
```

### Load Balancer Config

```nginx
upstream backend {
    server backend-1:8080;
    server backend-2:8080;
    server backend-3:8080;
}

server {
    listen 80;
    
    location /api/ {
        proxy_pass http://backend;
        proxy_set_header X-Request-ID $request_id;
    }
}
```

---

## 🔐 Security Checklist

- [ ] Change default PostgreSQL password
- [ ] Use HTTPS in production (Let's Encrypt)
- [ ] Enable CORS only for trusted domains
- [ ] Implement rate limiting
- [ ] Use strong JWT secrets
- [ ] Enable firewall rules
- [ ] Regular backups of PostgreSQL
- [ ] Monitor logs for suspicious activity
- [ ] Update Docker images regularly
- [ ] Use secrets management (HashiCorp Vault, etc.)

---

## 📊 Performance Tuning

### JVM Settings

```bash
# In docker-compose.yml or run command
JAVA_OPTS: "-Xmx2048m -Xms1024m -XX:+UseG1GC -XX:MaxGCPauseMillis=200"
```

### Database Optimization

```sql
-- Create indices
CREATE INDEX idx_conversation_id ON chat_messages(conversation_id);
CREATE INDEX idx_created_at ON chat_messages(created_at);
CREATE INDEX idx_model ON chat_messages(model_used);

-- Analyze tables
ANALYZE chat_messages;
```

### Ollama Optimization

```bash
# Set GPU memory
export OLLAMA_NUM_GPU=1
export OLLAMA_MAX_LOADED_MODELS=2

# Run with optimization
ollama serve
```

---

## 📚 Useful Commands

```bash
# Docker Commands
docker-compose up -d              # Start in background
docker-compose down               # Stop all services
docker-compose ps                 # Show status
docker-compose logs -f            # View logs
docker-compose exec backend sh    # SSH into container

# Database Backup
docker exec chatbot-postgres pg_dump \
  -U chatbot_user -d chatbot > backup.sql

# Database Restore
docker exec -i chatbot-postgres psql \
  -U chatbot_user -d chatbot < backup.sql

# Clean up
docker system prune -a            # Remove unused images
docker volume prune               # Remove unused volumes
```

---

## 📞 Support

- Backend API docs: `http://localhost:8080/swagger-ui.html`
- Frontend: `http://localhost:3000`
- Ollama: `http://localhost:11434`
- Database: `localhost:5432`
