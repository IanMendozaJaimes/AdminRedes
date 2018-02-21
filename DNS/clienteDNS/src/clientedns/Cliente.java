package clientedns;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.ArrayList;


public class Cliente implements Tipos{
    
    private final static int PUERTO = 53;
    private final static int MAX = 1024;
    
    public static void main(String args[]) throws IOException{
        
        while(true){
            
            DatagramSocket ds = new DatagramSocket(PUERTO);
            DatagramPacket dp = new DatagramPacket(new byte[MAX], MAX);
            
            ds.receive(dp);
            byte msg[] = dp.getData();
            
            ManejadorDNS mDNS = new ManejadorDNS();
            
            String dom[] = mDNS.obtenerDominoSolicitado(msg);

            System.out.println("DOMINIO: " + dom[0]);
            if(!dom[0].contains("facebook.com")){
                byte res[] = mDNS.mandarPreguntaDNS(msg, dp.getLength());
                DatagramPacket dpRes = new DatagramPacket(res, res.length, dp.getAddress(), dp.getPort());
                ds.send(dpRes);
            }
            else{
                if(dom[1].equals("1")){
                    byte res[] = mDNS.responder(msg, dp.getLength());
                    
                    for(int i = 0; i < res.length;){
                        String a = String.format("%x", res[i]);
                        String b = (a.length() == 1)?"0"+a:a;
                        System.out.print( b + " ");
                        if(++i % 16 == 0){
                            System.out.println("");
                        }
                        
                    }
                    
                    DatagramPacket dpRes = new DatagramPacket(res, res.length, dp.getAddress(), dp.getPort());
                    ds.send(dpRes);
                    System.out.println("\nENVIE MI CABECERA");
                }
                else{
                    byte res[] = mDNS.mandarPreguntaDNS(msg, dp.getLength());
                    DatagramPacket dpRes = new DatagramPacket(res, res.length, dp.getAddress(), dp.getPort());
                    ds.send(dpRes);
                }
            }
            
            ds.close();
            
        }
        
    }
    
}
