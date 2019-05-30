defmodule Server do
  use Application

  def start(_type, _args) do
    IO.puts("Server starting on ports 13000 and 13001")
    Task.start_link(Tcp, :start, [13000])
    Task.start_link(Router, :start, [])
    Task.start_link(Udp, :start, [13001])
  end
end

defmodule Tcp do
  @moduledoc """
  A module to open a TCP socket, accept any connection and
  echo back any binary message received.
  """

  @doc """
  Creates a TCP socket and starts listening for connections.
  """
  def start(port) do
    options = [:binary, reuseaddr: true, active: false, backlog: 5]
    {:ok, lsocket} = :gen_tcp.listen(port, options)
    accept_loop(lsocket)
  end

  @doc """
  Accept any incoming connection. We spawn a new process everytime.
  """
  def accept_loop(lsocket) do
    case :gen_tcp.accept(lsocket) do
      {:ok, socket} ->
        spawn(fn -> client_loop(socket) end)
        accept_loop(lsocket)
      {:error, reason} ->
        IO.puts "tcp accept failed on:"
        IO.inspect(reason)
    end
  end

  @doc """
  client_loop tries to receive as much data as possible and
  echoes it back to the sender.
  """
  def client_loop(socket) do
    case :gen_tcp.recv(socket, 0) do
      {:ok, message} ->
        case String.split(message, "--") do
          ["reg", nickname | _] -> Client.register_nick(nickname, socket)
          ["unreg", nickname | _] -> Client.register_nick(nickname, socket)
          ["pos", mynickname, posx, posy | _] -> Client.send_position(mynickname, posx, posy)
          ["chal", mynickname, nickname | _] -> Client.send_challenge(nickname, mynickname)
          ["choice", nickname, choice | _] -> Client.send_choice(nickname, choice)
          otherwise -> IO.inspect otherwise
        end
        client_loop(socket)
      otherwise ->
        IO.inspect(otherwise)
    end
  end

end

defmodule Udp do
  @moduledoc """
  Opens a UDP socket and use it to send back any message received.
  """

  @doc """
  Opens a UDP socket.
  """
  def start(port) do
    options = [:binary, reuseaddr: true, active: false]
    {:ok, socket} = :gen_udp.open(port, options)
    recv_loop(socket)
  end

  @doc """
  recv_loop receives as much data as possible and sends it
  back to the sender.
  """
  def recv_loop(socket) do
    case :gen_udp.recv(socket, 0) do
      {:ok, {message}}  ->
        IO.puts message
        #case String.split(message, "::") do
          #otherwise -> IO.inspect otherwise
        #end
        recv_loop(socket)
      {:error, reason} ->
        IO.puts "Udp server failed:"
        IO.inspect reason.
    end
  end
end
end

defmodule Router do

  @my_router :my_awesome_router

  # API
  def start() do
    pid = spawn(Router, :route_messages, [%{}])
    :global.register_name @my_router, pid
  end

  def stop() do
    :global.unregister(@my_router)
  end

  def send_chat_message(message_body, addressee) do

  end

  def unregister_nick(clients, nickname) do

  end

  def route_messages(clients) do
    receive do
      # Talk to different machines
      # Robustness
      # Distributing to clients directly
      {:send_choice, nickname, choice} ->
        case Map.get(clients, nickname) do
        nil ->
          IO.puts "Addressee #{nickname} unknown."
        clientvalue ->
          message = "choice #{choice} ::\n"
          socket = Map.get(clients, nickname)
          :gen_tcp.send socket[:client],  message
          IO.puts message
        end
        route_messages(clients)
      {:send_challenge, nickname, mynickname} ->
        case Map.get(clients, nickname) do
        nil ->
          IO.puts "Addressee #{nickname} unknown."
        clientvalue ->
          message = "chal #{mynickname}\n"
          socket = Map.get(clients, nickname)
          :gen_tcp.send socket[:client],  message
          IO.puts message
        end
        route_messages(clients)

      {:send_position, mynickname, posx, posy} ->
        case Map.get(clients, mynickname) do
          nil ->
            IO.puts " #{mynickname} unknown."
          socket ->
            new = [client: socket[:client], posX: posx, posY: posy]
            clients = Map.put(clients, mynickname, new)
            message = "#{mynickname} #{posx} #{posy}\n"
            values = Map.values(clients) -- [new] |> IO.inspect
            fun = fn(value) -> :gen_tcp.send value[:client], message end
            IO.puts message
            Enum.map values, fun
            route_messages(clients)
        end
      {:register_nick, nickname, socket} ->
        case Map.get(clients, nickname) do
          nil ->
            IO.puts "Registered #{nickname}."
  		        route_messages(Map.put(clients, nickname ,[client: socket, posX: 0, posY: 0]))
          found ->
            IO.puts "User Already Registered."
            send_chat_message("User Already Registered.",found)
        end
      {:unregister_nick, nickname} ->
        case Map.get(clients, nickname) do
          nil ->
            IO.puts "#{nickname} Doesnt Exist!"
          found ->
            IO.puts "User Un-Registered."
            route_messages(Map.delete(clients, nickname))
        end
      :shutdown ->
        IO.puts "Shutting down."
		stop()
      anything_else ->
        IO.puts "Warning! Received:"
        IO.inspect anything_else
        route_messages(clients)
    end
  end

end

defmodule Client do
  def send_choice(nickname, choice) do
    :global.send(:my_awesome_router,{:send_choice, nickname, choice})
  end

  def send_challenge(mynickname, nickname) do
    :global.send(:my_awesome_router,{:send_challenge, nickname, mynickname})
  end

  def send_position( mynickname,posx,posy) do
    :global.send(:my_awesome_router,{:send_position,mynickname,posx,posy})
  end

  def register_nick(nickname, socket) do
    :global.send(:my_awesome_router,{:register_nick, nickname, socket})
  end

  def unregister_nick(nickname) do
    :global.send(:my_awesome_router,{:register_nick, nickname})
  end



end
