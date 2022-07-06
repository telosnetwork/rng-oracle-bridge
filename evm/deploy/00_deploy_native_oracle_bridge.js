module.exports = async ({getNamedAccounts, deployments}) => {
    const {deploy} = deployments;
    const {deployer} = await getNamedAccounts();


    const bridge = await deploy('RNGOracleBridge', {
        from: deployer,
        args: ["1000000000000000000", 10, "0x6c33bdd2622e59fd10b411ff8d8d8d4dc5caf6ce"],
    });

    console.log("Deployed to:", bridge.address);
    const tester = await deploy('RNGOracleConsumer', {
        from: deployer,
        args: [bridge.address],
    });
    console.log("Deployed to:", tester.address);

};
module.exports.tags = ['RNGOracleBridge'];