FROM python:latest


RUN apt update

RUN apt install python3 -y
RUN pip install numpy
RUN pip install matplotlib
RUN pip install nats-py
RUN pip install asyncio
RUN pip install influxdb_client

WORKDIR /app

COPY . .


RUN mkdir -p /var/log/app.log && touch app.log

CMD ["python","./qlearning.py"]