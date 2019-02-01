FROM node:11

COPY package*.json /root/
RUN cd /root/ \
  && npm install

COPY src/ /root/src/
WORKDIR /root/
EXPOSE 8080
CMD ["npm", "start"]
