FROM node:11

COPY ./client/package*.json /root/client/
RUN cd /root/client/ \
  && npm install

COPY package*.json /root/
RUN cd /root/ \
  && npm install

COPY ./client/ /root/client/
ARG REACT_APP_LOCATION_NAME
RUN cd /root/client/ \
  && npm run build

COPY ./src/ /root/src/
EXPOSE 8080
WORKDIR /root/
CMD ["npm", "start"]
