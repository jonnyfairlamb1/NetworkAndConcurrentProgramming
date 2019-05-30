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
    :get_tcp.send addressee,  message_body
  end

  def register_nick(clients, nickname, socket) do
    route_messages(Map.put(clients, socket, nickname))
  end

  def unregister_nick(clients, nickname) do
    route_messages(Map.delete(clients, nickname))
  end

  def route_messages(clients) do
    receive do
      # Talk to different machines
      # Robustness
      # Distributing to clients directly
      {:send_chat_message, addressee, sender, message_body} ->
        case Map.get(clients, addressee) do
        nil ->
          IO.puts "Addressee #{addressee} unknown."
        clientvalue ->
		        send_chat_message(message_body,addressee)
        end
        route_messages(clients)
      {:register_nick, nickname, socket} ->
        case Map.get(clients, nickname) do
          nil ->
            IO.puts "Registered #{nickname}."
  		      Map.put(nickname, socket)
          found ->
            IO.puts "User Already Registered."
            send_chat_message("User Already Registered.",found)
        end
      {:unregister_nick, nickname} ->
       	unregister_nick(clients, nickname)
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
  def send_chat_message(message_body, addressee) do
    :global.send(:my_awesome_router,{:send_chat_message, addressee, self(), message_body})
  end

  def receive_msg(who) do
    receive do
      :stop ->
        IO.puts "Client #{who} closing"
      {:chat_msg, message_body} ->
        IO.puts "#{who} received #{message_body}"
        receive_msg(who)
    end
  end

  def register_nick(nickname) do
    pid = spawn(Client, :receive_msg, [nickname])
    :global.send(:my_awesome_router,{:register_nick, nickname, socket})
  end

  def unregister_nick(nickname) do
    :global.send(:my_awesome_router,{:register_nick, nickname})
  end



end
