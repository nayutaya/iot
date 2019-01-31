FROM node:11

COPY package*.json /root/
RUN cd /root/ \
  && npm install

COPY src/ /root/src/
WORKDIR /root/
CMD ["npm", "start"]
