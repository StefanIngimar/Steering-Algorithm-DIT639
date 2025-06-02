FROM python:3.11-slim

WORKDIR /app

COPY microservices/karen /app

RUN pip install --no-cache-dir -r requirements.txt

ENV PYTHONUNBUFFERED=1

CMD ["python3", "src/main.py"]

