FROM node:11

COPY package*.json /root/
RUN cd /root/ \
  && npm install

COPY index.js /root/
WORKDIR /root/
CMD ["npm", "start"]
