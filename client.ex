defmodule Client do
  def main() do
    options = [:binary, active: false]
    {:ok, socket} = :gen_tcp.connect({127,0,0,1}, 13000, options)
    :ok = :gen_tcp.send(socket, "I was there")
    case :gen_tcp.recv(socket, 0) do
      {:ok, msg}       -> IO.puts "TCP client received: #{msg}"
      {:error, reason} -> IO.inspect reason
    end
    :ok = :gen_tcp.close(socket)
    {:ok, socket} = :gen_udp.open(0,options)
    :ok = :gen_udp.send(socket, {127,0,0,1}, 13001, "Now I am here")
    case :gen_udp.recv(socket, 0) do
      {:ok, {_sender, _port, msg}} -> IO.puts "Udp client received: #{msg}"
      {:error, reason}             -> IO.inspect(reason)
    end
    :ok = :gen_udp.close(socket)
  end

end

