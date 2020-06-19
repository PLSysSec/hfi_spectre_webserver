wrk.host = "localhost"
wrk.port = "8000"
wrk.method = "GET"
wrk.path = "/echo_server"

local some_thread = {}

function init(args)
  assert(#args == 1)
  protection = args[1] -- lua is 1-indexed
  wrk.path = wrk.path .. "?protection=" .. protection
  wrk.path = wrk.path .. "&msg=hello"
end

function setup(thread)
  some_thread = thread
end

function done(summary, latency, requests)
  protection = some_thread:get("protection")
  avg_lat = latency.mean
  tail_lat = latency:percentile(99.0)
  throughput = summary.requests / summary.duration * 1000000  -- summary.duration is in microseconds
  file = io.open("results/echo_server_" .. protection .. ".txt", "w")
  file:write(string.format("Machine readable:\n%g\n%g\n%g\n\n", avg_lat, tail_lat, throughput))
  if avg_lat < 1000 then
    file:write(string.format("Avg latency: %g us\n", avg_lat))
  elseif avg_lat < 1000000 then
    file:write(string.format("Avg latency: %g ms\n", avg_lat/1000))
  else
    file:write(string.format("Avg latency: %g s\n", avg_lat/1000000))
  end
  if tail_lat < 1000 then
    file:write(string.format("99%% latency: %g us\n", tail_lat))
  elseif tail_lat < 1000000 then
    file:write(string.format("99%% latency: %g ms\n", tail_lat/1000))
  else
    file:write(string.format("99%% latency: %g s\n", tail_lat/1000000))
  end
  file:write(string.format("Throughput: %g req/s\n", throughput))
  file:close()
end
