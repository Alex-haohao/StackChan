ARG GO_IMAGE=golang:1.26.3-alpine
ARG ALPINE_IMAGE=alpine:3.22

FROM ${GO_IMAGE} AS build
WORKDIR /src

RUN apk add --no-cache ca-certificates

COPY go.mod go.sum ./
RUN go mod download all

COPY . .
RUN go mod tidy \
  && CGO_ENABLED=0 GOOS=linux go build -trimpath -ldflags="-s -w" -o /out/stackChan .

FROM ${ALPINE_IMAGE}
WORKDIR /app

RUN apk add --no-cache ca-certificates tzdata \
  && addgroup -S -g 10001 stackchan \
  && adduser -S -D -H -u 10001 -G stackchan -s /sbin/nologin stackchan

COPY --from=build /out/stackChan /app/stackChan
COPY --from=build /src/file /app/file

RUN mkdir -p /app/logs /app/manifest/config /app/web/management \
  && chown -R stackchan:stackchan /app

USER stackchan
EXPOSE 12800

CMD ["./stackChan"]
