# main.tf

provider "aws" {
  region = "us-east-2"  # Adjust region as needed
}

resource "aws_instance" "cluster" {
  count         = 200
  ami           = "ami-065d315e052507855"  # AWS Linux 2 AMI, adjust as needed
  instance_type = "t3.nano"
  key_name      = var.key_name  # Adjust with your SSH key name
  tags = {
    Name = "cluster-node-${count.index}"
  }
  
  instance_market_options {
    market_type = "spot"
  }

  user_data = <<-EOF
  		#!/bin/bash
		sudo apt-get update -y
		sudo apt-get install -y gcc g++ make libgmp-dev libmpfr-dev libmpc-dev libtool
		git clone https://github.com/davidsenack/bigdiv.git /home/ubuntu/bigdiv
		cd /home/ubuntu/bigdiv
		make
		./program 400 > /home/ubuntu/bigdiv/results.txt
}

output "public_ip_addresses" {
  value = aws_instance.cluster[*].public_ip
}
