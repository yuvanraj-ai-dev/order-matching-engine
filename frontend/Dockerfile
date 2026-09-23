# ---- Build stage -----------------------------------------------------------
FROM node:20-alpine AS build

WORKDIR /app
COPY package.json ./
RUN npm install

COPY . .

# Baked in at build time. Override with --build-arg when building the image
# (see README) to point at your deployed backend URL.
ARG VITE_API_URL=""
ARG VITE_WS_URL=""
ENV VITE_API_URL=$VITE_API_URL
ENV VITE_WS_URL=$VITE_WS_URL

RUN npm run build

# ---- Runtime stage -----------------------------------------------------------
FROM nginx:alpine AS runtime

COPY --from=build /app/dist /usr/share/nginx/html
COPY nginx.conf /etc/nginx/conf.d/default.conf

EXPOSE 80
CMD ["nginx", "-g", "daemon off;"]
