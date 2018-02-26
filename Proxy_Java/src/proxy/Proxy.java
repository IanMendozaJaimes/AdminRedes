package proxy;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;
import java.util.logging.Level;
import java.util.logging.Logger;


public class Proxy {
    
    private static final int MAX_BUFFER = 1024;
    private static final int PUERTO = 53;

    public static void main(String[] args) {
        
        System.out.println("Servidor escuchando en el puerto " + PUERTO + " ...");
        
        while(true){
            try{
                DatagramSocket ds = new DatagramSocket(PUERTO);
                DatagramPacket dp = new DatagramPacket(new byte[MAX_BUFFER], MAX_BUFFER);
                
                ds.receive(dp);
                
                ManejadorCliente mc = new ManejadorCliente(dp, MAX_BUFFER);
                mc.start();
                //mc.join();
                ds.close();
            }
            catch(SocketException e){
                e.printStackTrace();
            } catch (IOException ex) {
                Logger.getLogger(Proxy.class.getName()).log(Level.SEVERE, null, ex);
            }
        }

    }
    
}
